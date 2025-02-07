#include <LibHTML/DOM/Document.h>
#include <LibHTML/DOM/Element.h>
#include <LibHTML/DOM/Text.h>
#include <LibHTML/Dump.h>
#include <LibHTML/Layout/LayoutNode.h>
#include <LibHTML/Layout/LayoutText.h>
#include <stdio.h>

void dump_tree(const Node& node)
{
    static int indent = 0;
    for (int i = 0; i < indent; ++i)
        printf("  ");
    if (node.is_document()) {
        printf("*Document*\n");
    } else if (node.is_element()) {
        printf("<%s", static_cast<const Element&>(node).tag_name().characters());
        static_cast<const Element&>(node).for_each_attribute([](auto& name, auto& value) {
            printf(" %s=%s", name.characters(), value.characters());
        });
        printf(">\n");
    } else if (node.is_text()) {
        printf("\"%s\"\n", static_cast<const Text&>(node).data().characters());
    }
    ++indent;
    if (node.is_parent_node()) {
        static_cast<const ParentNode&>(node).for_each_child([](auto& child) {
            dump_tree(child);
        });
    }
    --indent;
}

void dump_tree(const LayoutNode& layout_node)
{
    static int indent = 0;
    for (int i = 0; i < indent; ++i)
        printf("  ");

    String tag_name;
    if (layout_node.is_anonymous())
        tag_name = "(anonymous)";
    else if (layout_node.node()->is_text())
        tag_name = "#text";
    else if (layout_node.node()->is_document())
        tag_name = "#document";
    else if (layout_node.node()->is_element())
        tag_name = static_cast<const Element&>(*layout_node.node()).tag_name();
    else
        tag_name = "???";

    printf("%s {%s} at (%d,%d) size %dx%d",
        layout_node.class_name(),
        tag_name.characters(),
        layout_node.rect().x(),
        layout_node.rect().y(),
        layout_node.rect().width(),
        layout_node.rect().height());
    if (layout_node.is_text())
        printf(" \"%s\"", static_cast<const LayoutText&>(layout_node).text().characters());
    printf("\n");
    ++indent;
    layout_node.for_each_child([](auto& child) {
        dump_tree(child);
    });
    --indent;
}
