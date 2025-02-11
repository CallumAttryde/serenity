#include <LibHTML/DOM/Element.h>
#include <LibHTML/DOM/Text.h>
#include <LibHTML/Parser/Parser.h>
#include <ctype.h>
#include <stdio.h>

static Retained<Element> create_element(const String& tag_name)
{
    return adopt(*new Element(tag_name));
}

static bool is_valid_in_attribute_name(char ch)
{
    return isalnum(ch) || ch == '_' || ch == '-';
}

static bool is_self_closing_tag(const String& tag_name)
{
    return tag_name == "area"
        || tag_name == "base"
        || tag_name == "br"
        || tag_name == "col"
        || tag_name == "embed"
        || tag_name == "hr"
        || tag_name == "img"
        || tag_name == "input"
        || tag_name == "link"
        || tag_name == "meta"
        || tag_name == "param"
        || tag_name == "source"
        || tag_name == "track"
        || tag_name == "wbr";
}

Retained<Document> parse(const String& html)
{
    Vector<Retained<ParentNode>> node_stack;

    auto doc = adopt(*new Document);
    node_stack.append(doc);

    enum class State {
        Free = 0,
        BeforeTagName,
        InTagName,
        InAttributeList,
        InAttributeName,
        BeforeAttributeValue,
        InAttributeValueNoQuote,
        InAttributeValueSingleQuote,
        InAttributeValueDoubleQuote,
    };

    auto state = State::Free;

    Vector<char, 256> text_buffer;

    Vector<char, 32> tag_name_buffer;

    Vector<Attribute> attributes;
    Vector<char, 256> attribute_name_buffer;
    Vector<char, 256> attribute_value_buffer;

    bool is_slash_tag = false;

    auto move_to_state = [&](State new_state) {
        if (new_state == State::BeforeTagName) {
            is_slash_tag = false;
            tag_name_buffer.clear();
            attributes.clear();
        }
        if (new_state == State::InAttributeName)
            attribute_name_buffer.clear();
        if (new_state == State::BeforeAttributeValue)
            attribute_value_buffer.clear();
        if (state == State::Free && !text_buffer.is_empty()) {
            auto text_node = adopt(*new Text(String::copy(text_buffer)));
            text_buffer.clear();
            node_stack.last()->append_child(text_node);
        }
        state = new_state;
        text_buffer.clear();
    };

    auto close_tag = [&] {
        if (node_stack.size() > 1)
            node_stack.take_last();
    };

    auto open_tag = [&] {
        auto new_element = create_element(String::copy(tag_name_buffer));
        tag_name_buffer.clear();
        new_element->set_attributes(move(attributes));
        node_stack.append(new_element);
        if (node_stack.size() != 1)
            node_stack[node_stack.size() - 2]->append_child(new_element);

        if (is_self_closing_tag(new_element->tag_name()))
            close_tag();
    };

    auto commit_tag = [&] {
        if (is_slash_tag)
            close_tag();
        else
            open_tag();
    };

    auto commit_attribute = [&] {
        attributes.append({ String::copy(attribute_name_buffer), String::copy(attribute_value_buffer) });
    };

    for (int i = 0; i < html.length(); ++i) {
        char ch = html[i];
        switch (state) {
        case State::Free:
            if (ch == '<') {
                is_slash_tag = false;
                move_to_state(State::BeforeTagName);
                break;
            }
            text_buffer.append(ch);
            break;
        case State::BeforeTagName:
            if (ch == '/') {
                is_slash_tag = true;
                break;
            }
            if (ch == '>') {
                move_to_state(State::Free);
                break;
            }
            if (!isalpha(ch))
                break;
            move_to_state(State::InTagName);
            [[fallthrough]];
        case State::InTagName:
            if (isspace(ch)) {
                move_to_state(State::InAttributeList);
                break;
            }
            if (ch == '>') {
                commit_tag();
                move_to_state(State::Free);
                break;
            }
            tag_name_buffer.append(ch);
            break;
        case State::InAttributeList:
            if (ch == '>') {
                commit_tag();
                move_to_state(State::Free);
                break;
            }
            if (!isalpha(ch))
                break;
            move_to_state(State::InAttributeName);
            [[fallthrough]];
        case State::InAttributeName:
            if (is_valid_in_attribute_name(ch)) {
                attribute_name_buffer.append(ch);
                break;
            }
            if (isspace(ch)) {
                commit_attribute();
                break;
            }

            if (ch == '>') {
                commit_tag();
                move_to_state(State::Free);
                break;
            }

            if (ch == '=') {
                move_to_state(State::BeforeAttributeValue);
                break;
            }
            break;
        case State::BeforeAttributeValue:
            if (ch == '\'') {
                move_to_state(State::InAttributeValueSingleQuote);
                break;
            }
            if (ch == '"') {
                move_to_state(State::InAttributeValueDoubleQuote);
                break;
            }
            if (ch == '>') {
                commit_tag();
                move_to_state(State::Free);
                break;
            }
            if (isspace(ch)) {
                commit_attribute();
                move_to_state(State::InAttributeList);
                break;
            }
            break;
        case State::InAttributeValueSingleQuote:
            if (ch == '\'') {
                commit_attribute();
                move_to_state(State::InAttributeList);
                break;
            }
            attribute_value_buffer.append(ch);
            break;
        case State::InAttributeValueDoubleQuote:
            if (ch == '"') {
                commit_attribute();
                move_to_state(State::InAttributeList);
                break;
            }
            attribute_value_buffer.append(ch);
            break;
        case State::InAttributeValueNoQuote:
            if (isspace(ch)) {
                commit_attribute();
                move_to_state(State::InAttributeList);
                break;
            }
            if (ch == '>') {
                commit_tag();
                move_to_state(State::Free);
                break;
            }
            attribute_value_buffer.append(ch);
            break;
        default:
            fprintf(stderr, "Unhandled state %d\n", (int)state);
            ASSERT_NOT_REACHED();
        }
    }
    return doc;
}
