#include <api/client.h>

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <QVariantMap>
#include <QDebug>

namespace http = core::net::http;
namespace net = core::net;

using namespace api;
using namespace std;

// http://open.weibo.com/wiki/2/statuses/home_timeline
const QString BASE_URI = "https://api.weibo.com/2/statuses/home_timeline.json?access_token=%1&page=1";

Client::Client(Config::Ptr config) :
    config_(config), cancelled_(false) {
}

void Client::get( QString uri, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    configuration.uri = uri.toStdString();

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Client::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());

        qDebug() << "response: " << response.body.c_str();
    } catch (net::Error &) {
    }
}

Client::DataList Client::getData(const string& query) {
    QJsonDocument root;

    QString temp = QString::fromStdString(query);
    QByteArray bytearray = query.c_str();
    QString query_string = QString::fromUtf8(bytearray.data(), bytearray.size());

    qDebug() << "query_string: " << query_string;

    QString uri = BASE_URI.arg(m_accessToken);
    qDebug() << "uri: "  << uri;
    get(uri, root);

    DataList result;

    QVariantMap variant = root.toVariant().toMap();

    // Iterate through the weather data
    for (const QVariant &i : variant["statuses"].toList()) {
        QVariantMap item = i.toMap();

        QString created_at = item["created_at"].toString();
        qDebug() << "created at: " << created_at;

        // <a href=\"http://app.weibo.com/t/feed/3auC5p\" rel=\"nofollow\">皮皮时光机</a>
        QString source = item["source"].toString();
//        qDebug() << "source1: " << source;
        // QRegExp rex("<a .*>");

        // QRegExp rex("/<a href=\"(.*)\">(.*)<\/a>/");
        QRegExp rex(QString("<a href=\"(.*)\""));
        rex.setMinimal(true);
        int pos = 0;

        while( (pos = source.indexOf(rex, pos )) != -1 ){
            qDebug() <<"pos: " << pos << rex.cap(1);
            source = rex.cap(1);
            break;
            // pos += rex.matchedLength();
        }

        //        source = source.replace(rex,"");
        //        source= source.replace("</a>","");
        qDebug() << "source: " << source;

        QString text = item["text"].toString();
        qDebug() << "text: "  << text;

        QVariantMap user = item["user"].toMap();

        QString name = user["screen_name"].toString();
        qDebug() << "name: " << name;

        QString profile_image_url = user["profile_image_url"].toString();
        qDebug() << "profile_image_url: " << profile_image_url;

        // Add a result to the weather list
        result.emplace_back(
            Data { created_at.toStdString(), text.toStdString(), source.toStdString(),
                   name.toStdString(), profile_image_url.toStdString() });
    }

    return result;
}

http::Request::Progress::Next Client::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Client::cancel() {
    cancelled_ = true;
}

Config::Ptr Client::config() {
    return config_;
}

void Client::setAccessToken(QString accessToken) {
    m_accessToken = accessToken;
}
