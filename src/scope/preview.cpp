#include <scope/preview.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>
#include <QString>

namespace sc = unity::scopes;

using namespace std;
using namespace scope;

using namespace unity::scopes;

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    // Support three different column layouts
    sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);

    // We define 3 different layouts, that will be used depending on the
    // device. The shell (view) will decide which layout fits best.
    // If, for instance, we are executing in a tablet probably the view will use
    // 2 or more columns.
    // Column layout definitions are optional.
    // However, we recommend that scopes define layouts for the best visual appearance.

    // Single column layout
    layout1col.add_column( { "image", "header", "summary", "actions" });

    // Two column layout
    layout2col.add_column( { "image" });
    layout2col.add_column( { "header", "summary", "actions" });

    // Three cokumn layout
    layout3col.add_column( { "image" });
    layout3col.add_column( { "header", "summary", "actions" });
    layout3col.add_column( { });

    // Register the layouts we just created
    reply->register_layout( { layout1col, layout2col, layout3col });

    // Define the header section
    sc::PreviewWidget w_header("header", "header");
    // It has title and a subtitle properties
    w_header.add_attribute_mapping("title", "title");
    w_header.add_attribute_mapping("subtitle", "subtitle");

    // Define the image section
    sc::PreviewWidget w_image("image", "image");
    // It has a single source property, mapped to the result's art property
    w_image.add_attribute_mapping("source", "art");

    // Define the summary section
    sc::PreviewWidget w_description("summary", "text");
    // It has a text property, mapped to the result's description property
    w_description.add_attribute_mapping("text", "description");

    Result result = PreviewQueryBase::result();
    QString urlString(result["uri"].get_string().c_str());

    // Create an Open button and provide the URI to open for this preview result
    sc::PreviewWidget w_actions("actions", "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
            {"id", Variant("open")},
            {"label", Variant("Open")},
            {"uri", sc::Variant(urlString.toStdString())} // uri set, this action will be handled by the Dash
        });
    w_actions.add_attribute_value("actions", builder.end());

    PreviewWidgetList widgets({ w_header, w_image, w_description, w_actions});

    // Push each of the sections
    reply->push( widgets );
}

