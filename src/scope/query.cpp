#include <boost/algorithm/string/trim.hpp>

#include <scope/localization.h>
#include <scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/OnlineAccountClient.h>

#include <iomanip>
#include <sstream>

#include <QDebug>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace api;
using namespace scope;
using namespace sc;

const static string SEARCH_CATEGORY_LOGIN_NAG = R"(
{
        "schema-version": 1,
        "template": {
        "category-layout": "grid",
        "card-size": "large",
        "card-background": "color:///#1ab7ea"
        },
        "components": {
        "title": "title",
        "background": "background",
        "art" : {
        "aspect-ratio": 100.0
        }
        }
        }
        )";

/**
 * Define the layout for the forecast results
 *
 * The icon size is small, and ask for the card layout
 * itself to be horizontal. I.e. the text will be placed
 * next to the image.
 */
const static string WEATHER_TEMPLATE =
        R"(
{
        "schema-version": 1,
        "template": {
        "category-layout": "grid",
        "card-layout": "horizontal",
        "card-size": "small"
        },
        "components": {
        "title": "title",
        "art" : {
        "field": "art"
        },
        "subtitle": "subtitle"
        }
        }
        )";

/**
 * Define the larger "current weather" layout.
 *
 * The icons are larger.
 */
const static string CITY_TEMPLATE =
        R"(
{
        "schema-version": 1,
        "template": {
        "category-layout": "grid",
        "card-size": "medium"
        },
        "components": {
        "title": "title",
        "art" : {
        "field": "art"
        },
        "subtitle": "subtitle"
        }
        }
        )";

// Create a JSON string to be used tro create a category renderer - uses grid layout
const std::string CR_GRID = R"(
{
        "schema-version" : 1,
        "template" : {
            "category-layout" : "vertical-journal",
            "card-layout": "horizontal",
            "card-size": "small",
            "collapsed-rows": 0
        },
        "components" : {
            "title":"title",
            "subtitle":"subtitle",
            "summary":"summary",
            "art":{
                "field": "art2",
                "aspect-ratio": 1
            }
        }
})";

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             Config::Ptr config) :
    sc::SearchQueryBase(query, metadata), client_(config) {
}

void Query::cancelled() {
    client_.cancel();
}


void Query::run(sc::SearchReplyProxy const& reply) {
    add_login_nag(reply);

    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());
        QString queryString = QString::fromStdString(query.query_string());

        // Trim the query string of whitespace
        string query_string = alg::trim_copy(query.query_string());

#if 1
        Client::DataList datalist;
        if (query_string.empty()) {
            queryString = QString("");
            datalist = client_.getData(queryString.toStdString());
        } else {
            // otherwise, get the forecast for the search string
            datalist = client_.getData(query_string);
        }

        CategoryRenderer rdrGrid(CR_GRID);

        QString title = queryString;

        // Register two categories
        auto grid = reply->register_category("weibo", "", "", rdrGrid);

        // For each of the entry in the datalist
        for (const Client::Data &data : datalist) {

            // for each result
            const std::shared_ptr<const Category> * category;

            category = &grid;

            //create our categorized result using our pointer, which is either to out
            //grid or our carousel Category
            CategorisedResult catres((*category));

            // We must have a URI

            catres.set_uri(data.source);
            catres.set_dnd_uri(data.source);
            catres.set_title(data.name);

            catres["subtitle"] = data.text;
//            catres["summary"] = data.summary;
//            catres["fulldesc"] = data.summary;
            catres["art2"] = data.profile_image_url;

            catres.set_art(data.profile_image_url);

//            catres["address"] = Variant(data.address);
//            catres["telephone"] = Variant(data.telephone);

            // Push the result
            if (!reply->push(catres)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
        }
#endif

    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}

void Query::add_login_nag(const sc::SearchReplyProxy &reply) {
    //    if (getenv("VIMEO_SCOPE_IGNORE_ACCOUNTS")) {
    //        return;
    //    }
    qDebug() << "SCOPE_INSTALL_NAME: " << SCOPE_INSTALL_NAME;
    qDebug() << "SCOPE_ACCOUNTS_NAME: " << SCOPE_ACCOUNTS_NAME;

    sc::OnlineAccountClient oa_client(SCOPE_INSTALL_NAME, "sharing", SCOPE_ACCOUNTS_NAME);
    // sc::OnlineAccountClient oa_client("weibo.ubuntu_weibo",
    //                                  "sharing", "weibo.ubuntu_accounts");

//    bool include_login_nag = !client_.authenticated();
//    qDebug() << "include_login_nag" << include_login_nag;

    // Check if our service is authenticated
    bool service_authenticated = false;

    int count = oa_client.get_service_statuses().size();
    qDebug() << "count: " << count;

    for ( sc::OnlineAccountClient::ServiceStatus const& status :
          oa_client.get_service_statuses())
    {
        if (status.service_authenticated)
        {
            service_authenticated = true;
            qDebug() << "Sevice is authenticated!";
            qDebug() << "account id: "  << status.account_id;
            qDebug() << "client id: " << QString::fromStdString(status.client_id);
            qDebug() << "service enabled: " << status.service_enabled;
            qDebug() << "secret: " << QString::fromStdString(status.client_secret);
            qDebug() << "access token: " << QString::fromStdString(status.access_token);
            accessToken_ = QString::fromStdString(status.access_token);

            // pass the access token to the client so that http request can be made
            client_.setAccessToken(accessToken_);
            break;
        }
    }

    if (!service_authenticated)
    {
        qDebug() << "Service is not authenicated!";

        sc::CategoryRenderer rdr(SEARCH_CATEGORY_LOGIN_NAG);
        auto cat = reply->register_category("weibo_login_nag", "", "", rdr);

        sc::CategorisedResult res(cat);
        res.set_title(_("Log-in to Weibo"));

        oa_client.register_account_login_item(res,
                                              query(),
                                              sc::OnlineAccountClient::InvalidateResults,
                                              sc::OnlineAccountClient::DoNothing);

        reply->push(res);
    }
}

