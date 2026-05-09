// Main public API for cmark-gfm markdown parser (C++ modernization)
// Purpose: Primary include for users - provides markdown parsing, manipulation, and rendering
// Key types: cmark_node (AST), cmark_parser, cmark_iter, cmark_syntax_extension
// Key functions: cmark_parse_document(), cmark_render_html(), node manipulation APIs
// Modernization: C++ types, trailing return syntax, enum class where possible
// Original: BSD 2-Clause license (see cppmark/licenses/LICENSE_cmark)
// Dependencies: ../include/cppmark_html.h for public API types

/////////////////////////////////////////////////////////////////////////////
// Purpose:   CommonMark parsing, manipulating, and rendering
// Authors:   John MacFarlane, Vicent Marti, Kārlis Gaņģis, Nick Wellnhofer
// License:   FreeBSD -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// C++ Modifications: Randalphwa

#pragma once

#include <cstdint>
#include <cstdio>
#include <string>

#include "../include/cppmark_html.h"

// ## Node Structure

#define CMARK_NODE_TYPE_PRESENT (0x8000)
#define CMARK_NODE_TYPE_BLOCK   (CMARK_NODE_TYPE_PRESENT | 0x0000)
#define CMARK_NODE_TYPE_INLINE  (CMARK_NODE_TYPE_PRESENT | 0x4000)
#define CMARK_NODE_TYPE_MASK    (0xc000)
#define CMARK_NODE_VALUE_MASK   (0x3fff)

typedef enum
{
    // Error status
    CMARK_NODE_NONE = 0x0000,

    // Block
    CMARK_NODE_DOCUMENT = CMARK_NODE_TYPE_BLOCK | 0x0001,
    CMARK_NODE_BLOCK_QUOTE = CMARK_NODE_TYPE_BLOCK | 0x0002,
    CMARK_NODE_LIST = CMARK_NODE_TYPE_BLOCK | 0x0003,
    CMARK_NODE_ITEM = CMARK_NODE_TYPE_BLOCK | 0x0004,
    CMARK_NODE_CODE_BLOCK = CMARK_NODE_TYPE_BLOCK | 0x0005,
    CMARK_NODE_HTML_BLOCK = CMARK_NODE_TYPE_BLOCK | 0x0006,
    CMARK_NODE_CUSTOM_BLOCK = CMARK_NODE_TYPE_BLOCK | 0x0007,
    CMARK_NODE_PARAGRAPH = CMARK_NODE_TYPE_BLOCK | 0x0008,
    CMARK_NODE_HEADING = CMARK_NODE_TYPE_BLOCK | 0x0009,
    CMARK_NODE_THEMATIC_BREAK = CMARK_NODE_TYPE_BLOCK | 0x000a,
    CMARK_NODE_FOOTNOTE_DEFINITION = CMARK_NODE_TYPE_BLOCK | 0x000b,

    // Inline
    CMARK_NODE_TEXT = CMARK_NODE_TYPE_INLINE | 0x0001,
    CMARK_NODE_SOFTBREAK = CMARK_NODE_TYPE_INLINE | 0x0002,
    CMARK_NODE_LINEBREAK = CMARK_NODE_TYPE_INLINE | 0x0003,
    CMARK_NODE_CODE = CMARK_NODE_TYPE_INLINE | 0x0004,
    CMARK_NODE_HTML_INLINE = CMARK_NODE_TYPE_INLINE | 0x0005,
    CMARK_NODE_CUSTOM_INLINE = CMARK_NODE_TYPE_INLINE | 0x0006,
    CMARK_NODE_EMPH = CMARK_NODE_TYPE_INLINE | 0x0007,
    CMARK_NODE_STRONG = CMARK_NODE_TYPE_INLINE | 0x0008,
    CMARK_NODE_LINK = CMARK_NODE_TYPE_INLINE | 0x0009,
    CMARK_NODE_IMAGE = CMARK_NODE_TYPE_INLINE | 0x000a,
    CMARK_NODE_FOOTNOTE_REFERENCE = CMARK_NODE_TYPE_INLINE | 0x000b,
} cmark_node_type;

extern cmark_node_type CMARK_NODE_LAST_BLOCK;
extern cmark_node_type CMARK_NODE_LAST_INLINE;

// TODO: [Randalphwa - 07-04-2025] This is now C++ code, so there is nothing
// to be backwards compatible with.

// For backwards compatibility:
#define CMARK_NODE_HEADER      CMARK_NODE_HEADING
#define CMARK_NODE_HRULE       CMARK_NODE_THEMATIC_BREAK
#define CMARK_NODE_HTML        CMARK_NODE_HTML_BLOCK
#define CMARK_NODE_INLINE_HTML CMARK_NODE_HTML_INLINE

typedef enum
{
    CMARK_NO_LIST,
    CMARK_BULLET_LIST,
    CMARK_ORDERED_LIST
} cmark_list_type;

typedef enum
{
    CMARK_NO_DELIM,
    CMARK_PERIOD_DELIM,
    CMARK_PAREN_DELIM
} cmark_delim_type;

struct cmark_node;
struct cmark_parser;
struct cmark_iter;
struct cmark_syntax_extension;

// ## Basic data structures

// To keep dependencies to the strict minimum, libcmark implements its own versions of "classic"
// data structures.

// ### Linked list

// A generic singly linked list.
struct cmark_llist
{
    cmark_llist* next;
    void* data;
};

// Append an element to the linked list, return the possibly modified head of the list.
auto cmark_llist_append(cmark_llist* head, void* data) -> cmark_llist*;

// Callback for freeing user data.
using cmark_free_func = void (*)(void* user_data);

// Free the list starting with 'head', calling 'free_func' with the data pointer of each of its
// elements
void cmark_llist_free_full(cmark_llist* head, cmark_free_func free_func);

// Free the list starting with 'head'
void cmark_llist_free(cmark_llist* head);

// ## Creating and Destroying Nodes

// Creates a new node of type 'type'.  Note that the node may have other required properties, which
// it is the caller's responsibility to assign.
auto cmark_node_new(cmark_node_type type) -> cmark_node*;

auto cmark_node_new_with_ext(cmark_node_type type, cmark_syntax_extension* extension)
    -> cmark_node*;

auto cmark_node_new_with_mem_and_ext(cmark_node_type type, cmark_syntax_extension* extension)
    -> cmark_node*;

// Frees the memory allocated for a node and any children.
void cmark_node_free(cmark_node* node);

// ## Tree Traversal

// Returns the next node in the sequence after 'node', or NULL if there is none.
auto cmark_node_next(cmark_node* node) -> cmark_node*;

// Returns the previous node in the sequence after 'node', or NULL if there is none.
auto cmark_node_previous(cmark_node* node) -> cmark_node*;

// Returns the parent of 'node', or NULL if there is none.
auto cmark_node_parent(cmark_node* node) -> cmark_node*;

// Returns the first child of 'node', or NULL if 'node' has no children.
auto cmark_node_first_child(cmark_node* node) -> cmark_node*;

// Returns the last child of 'node', or NULL if 'node' has no children.
auto cmark_node_last_child(cmark_node* node) -> cmark_node*;

// Returns the footnote reference of 'node', or NULL if 'node' doesn't have a footnote reference.
auto cmark_node_parent_footnote_def(cmark_node* node) -> cmark_node*;

/**
 * ## Iterator
 *
 * An iterator will walk through a tree of nodes, starting from a root
 * node, returning one node at a time, together with information about
 * whether the node is being entered or exited.  The iterator will
 * first descend to a child node, if there is one.  When there is no
 * child, the iterator will go to the next sibling.  When there is no
 * next sibling, the iterator will return to the parent (but with
 * a 'cmark_event_type' of `CMARK_EVENT_EXIT`).  The iterator will
 * return `CMARK_EVENT_DONE` when it reaches the root node again.
 * One natural application is an HTML renderer, where an `ENTER` event
 * outputs an open tag and an `EXIT` event outputs a close tag.
 * An iterator might also be used to transform an AST in some systematic
 * way, for example, turning all level-3 headings into regular paragraphs.
 *
 *     void
 *     usage_example(cmark_node *root) {
 *         cmark_event_type ev_type;
 *         cmark_iter *iter = cmark_iter_new(root);
 *
 *         while ((ev_type = cmark_iter_next(iter)) != CMARK_EVENT_DONE) {
 *             cmark_node *cur = cmark_iter_get_node(iter);
 *             // Do something with `cur` and `ev_type`
 *         }
 *
 *         cmark_iter_free(iter);
 *     }
 *
 * Iterators will never return `EXIT` events for leaf nodes, which are nodes
 * of type:
 *
 * * CMARK_NODE_HTML_BLOCK
 * * CMARK_NODE_THEMATIC_BREAK
 * * CMARK_NODE_CODE_BLOCK
 * * CMARK_NODE_TEXT
 * * CMARK_NODE_SOFTBREAK
 * * CMARK_NODE_LINEBREAK
 * * CMARK_NODE_CODE
 * * CMARK_NODE_HTML_INLINE
 *
 * Nodes must only be modified after an `EXIT` event, or an `ENTER` event for
 * leaf nodes.
 */

typedef enum
{
    CMARK_EVENT_NONE,
    CMARK_EVENT_DONE,
    CMARK_EVENT_ENTER,
    CMARK_EVENT_EXIT
} cmark_event_type;

// Creates a new iterator starting at 'root'.  The current node and event type are undefined until
// 'cmark_iter_next' is called for the first time. The memory allocated for the iterator should be
// released using 'cmark_iter_free' when it is no longer needed.
auto cmark_iter_new(cmark_node* root) -> cmark_iter*;

// Frees the memory allocated for an iterator.
void cmark_iter_free(cmark_iter* iter);

// Advances to the next node and returns the event type (`CMARK_EVENT_ENTER`, `CMARK_EVENT_EXIT` or
// `CMARK_EVENT_DONE`).
auto cmark_iter_next(cmark_iter* iter) -> cmark_event_type;

// Returns the current node.
auto cmark_iter_get_node(cmark_iter* iter) -> cmark_node*;

// Returns the current event type.
auto cmark_iter_get_event_type(cmark_iter* iter) -> cmark_event_type;

// Returns the root node.
auto cmark_iter_get_root(cmark_iter* iter) -> cmark_node*;

// Resets the iterator so that the current node is 'current' and the event type is 'event_type'.
// The new current node must be a descendant of the root node or the root node itself.
void cmark_iter_reset(cmark_iter* iter, cmark_node* current, cmark_event_type event_type);

// ## Accessors

// Returns the user data of 'node'.
auto cmark_node_get_user_data(cmark_node* node) -> void*;

// Sets arbitrary user data for 'node'.  Returns 1 on success, 0 on failure.
auto cmark_node_set_user_data(cmark_node* node, void* user_data) -> int;

// Set free function for user data

auto cmark_node_set_user_data_free_func(cmark_node* node, cmark_free_func free_func) -> int;

// Returns the type of 'node', or `CMARK_NODE_NONE` on error.
auto cmark_node_get_type(cmark_node* node) -> cmark_node_type;

// Like 'cmark_node_get_type', but returns a string representation of the type, or `"<unknown>"`.
auto cmark_node_get_type_string(cmark_node* node) -> const char*;

// Returns the string contents of 'node', or an empty string if none is set.  Returns NULL if called
// on a node that does not have string content.
auto cmark_node_get_literal(cmark_node* node) -> const char*;

// Sets the string contents of 'node'.  Returns 1 on success, 0 on failure.
auto cmark_node_set_literal(cmark_node* node, const char* content) -> int;

// Returns the heading level of 'node', or 0 if 'node' is not a heading.
auto cmark_node_get_heading_level(cmark_node* node) -> int;

// TODO: [Randalphwa - 07-04-2025] This is now C++ code, so there is nothing
// for backwards compatibility
#define cmark_node_get_header_level cmark_node_get_heading_level
#define cmark_node_set_header_level cmark_node_set_heading_level

// Sets the heading level of 'node', returning 1 on success and 0 on error.
auto cmark_node_set_heading_level(cmark_node* node, int level) -> int;

// Returns the list type of 'node', or `CMARK_NO_LIST` if 'node' is not a list.
auto cmark_node_get_list_type(cmark_node* node) -> cmark_list_type;

// Sets the list type of 'node', returning 1 on success and 0 on error.
auto cmark_node_set_list_type(cmark_node* node, cmark_list_type type) -> int;

// Returns the list delimiter type of 'node', or `CMARK_NO_DELIM` if 'node' is not a list.
auto cmark_node_get_list_delim(cmark_node* node) -> cmark_delim_type;

// Sets the list delimiter type of 'node', returning 1 on success and 0
// on error.
auto cmark_node_set_list_delim(cmark_node* node, cmark_delim_type delim) -> int;

// Returns starting number of 'node', if it is an ordered list, otherwise 0.
auto cmark_node_get_list_start(cmark_node* node) -> int;

// Sets starting number of 'node', if it is an ordered list. Returns 1 on success, 0 on failure.
int cmark_node_set_list_start(cmark_node* node, int start);

// Returns 1 if 'node' is a tight list, 0 otherwise.
int cmark_node_get_list_tight(cmark_node* node);

// Sets the "tightness" of a list.  Returns 1 on success, 0 on failure.
int cmark_node_set_list_tight(cmark_node* node, int tight);

// Returns item index of 'node'. This is only used when rendering output formats such as commonmark,
// which need to output the index. It is not required for formats such as html or latex.
int cmark_node_get_item_index(cmark_node* node);

// Sets item index of 'node'. Returns 1 on success, 0 on failure.
int cmark_node_set_item_index(cmark_node* node, int idx);

// Returns the info string from a fenced code block.
const char* cmark_node_get_fence_info(cmark_node* node);

// Sets the info string in a fenced code block, returning 1 on success and 0 on failure.
int cmark_node_set_fence_info(cmark_node* node, const char* info);

// Sets code blocks fencing details
int cmark_node_set_fenced(cmark_node* node, int fenced, int length, int offset, char character);

// Returns code blocks fencing details
int cmark_node_get_fenced(cmark_node* node, int* length, int* offset, char* character);

// Returns the URL of a link or image 'node', or an empty string if no URL is set.  Returns NULL if
// called on a node that is not a link or image.
const char* cmark_node_get_url(cmark_node* node);

// Sets the URL of a link or image 'node'. Returns 1 on success, 0 on failure.
int cmark_node_set_url(cmark_node* node, const char* url);

// Returns the title of a link or image 'node', or an empty string if no title is set.  Returns NULL
// if called on a node that is not a link or image.
const char* cmark_node_get_title(cmark_node* node);

// Sets the title of a link or image 'node'. Returns 1 on success, 0 on failure.
int cmark_node_set_title(cmark_node* node, const char* title);

// Returns the literal "on enter" text for a custom 'node', or an empty string if no on_enter is
// set.  Returns NULL if called on a non-custom node.
const char* cmark_node_get_on_enter(cmark_node* node);

// Sets the literal text to render "on enter" for a custom 'node'. Any children of the node will be
// rendered after this text. Returns 1 on success 0 on failure.
int cmark_node_set_on_enter(cmark_node* node, const char* on_enter);

// Returns the literal "on exit" text for a custom 'node', or an empty string if no on_exit is set.
// Returns NULL if called on a non-custom node.
const char* cmark_node_get_on_exit(cmark_node* node);

// Sets the literal text to render "on exit" for a custom 'node'. Any children of the node will be
// rendered before this text. Returns 1 on success 0 on failure.
int cmark_node_set_on_exit(cmark_node* node, const char* on_exit);

// Returns the line on which 'node' begins.
int cmark_node_get_start_line(cmark_node* node);

// Returns the column at which 'node' begins.
int cmark_node_get_start_column(cmark_node* node);

// Returns the line on which 'node' ends.
int cmark_node_get_end_line(cmark_node* node);

// Returns the column at which 'node' ends.
int cmark_node_get_end_column(cmark_node* node);

// ## Tree Manipulation

// Unlinks a 'node', removing it from the tree, but not freeing its memory.  (Use 'cmark_node_free'
// for that.)
void cmark_node_unlink(cmark_node* node);

// Inserts 'sibling' before 'node'.  Returns 1 on success, 0 on failure.
int cmark_node_insert_before(cmark_node* node, cmark_node* sibling);

// Inserts 'sibling' after 'node'. Returns 1 on success, 0 on failure.
int cmark_node_insert_after(cmark_node* node, cmark_node* sibling);

// Replaces 'oldnode' with 'newnode' and unlinks 'oldnode' (but does not free its memory). Returns 1
// on success, 0 on failure.
int cmark_node_replace(cmark_node* oldnode, cmark_node* newnode);

// Adds 'child' to the beginning of the children of 'node'. Returns 1 on success, 0 on failure.
int cmark_node_prepend_child(cmark_node* node, cmark_node* child);

// Adds 'child' to the end of the children of 'node'. Returns 1 on success, 0 on failure.
int cmark_node_append_child(cmark_node* node, cmark_node* child);

// Consolidates adjacent text nodes.
void cmark_consolidate_text_nodes(cmark_node* root);

// Ensures a node and all its children own their own chunk memory.
void cmark_node_own(cmark_node* root);

/**
 * ## Parsing
 *
 * Simple interface:
 *
 *     cmark_node *document = cmark_parse_document("Hello *world*", 13,
 *                                                 CMARK_OPT_DEFAULT);
 *
 * Streaming interface:
 *
 *     cmark_parser *parser = cmark_parser_new(CMARK_OPT_DEFAULT);
 *     FILE *fp = fopen("myfile.md", "rb");
 *     while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
 *     	   cmark_parser_feed(parser, buffer, bytes);
 *     	   if (bytes < sizeof(buffer)) {
 *     	       break;
 *     	   }
 *     }
 *     document = cmark_parser_finish(parser);
 *     cmark_parser_free(parser);
 */

// Creates a new parser object.
cmark_parser* cmark_parser_new(int options);

// Frees memory allocated for a parser object.
void cmark_parser_free(cmark_parser* parser);

// Feeds a string of length 'len' to 'parser'.
void cmark_parser_feed(cmark_parser* parser, const char* buffer, size_t len);

// Finish parsing and return a pointer to a tree of nodes.
cmark_node* cmark_parser_finish(cmark_parser* parser);

// Parse a CommonMark document in 'buffer' of length 'len'. Returns a pointer to a tree of nodes.
// The memory allocated for the node tree should be released using 'cmark_node_free' when it is no
// longer needed.
cmark_node* cmark_parse_document(const char* buffer, size_t len, int options);

// Parse a CommonMark document in file 'file', returning a pointer to a tree of nodes.  The memory
// allocated for the node tree should be released using 'cmark_node_free' when it is no longer
// needed.
cmark_node* cmark_parse_file(FILE* file, int options);

// ## Rendering

// Render a 'node' tree as XML.  It is the caller's responsibility to free the returned buffer.
char* cmark_render_xml(cmark_node* root, int options);

// Render a 'node' tree as an HTML fragment. It is up to the user
// to add an appropriate header and footer.
std::string cmark_render_html(cmark_node* root, int options, cmark_llist* extensions);

// Render a 'node' tree as a groff man page, without the header. It is the caller's responsibility
// to free the returned buffer.
char* cmark_render_man(cmark_node* root, int options, int width);

// Render a 'node' tree as a commonmark document. It is the caller's responsibility to free the
// returned buffer.
char* cmark_render_commonmark(cmark_node* root, int options, int width);

// Render a 'node' tree as a plain text document. It is the caller's responsibility to free the
// returned buffer.
char* cmark_render_plaintext(cmark_node* root, int options, int width);

// Render a 'node' tree as a LaTeX document. It is the caller's responsibility to free the returned
// buffer.
char* cmark_render_latex(cmark_node* root, int options, int width);

// ## Version information

/** The library version as integer for runtime checks. Also available as
 * macro CMARK_VERSION for compile time checks.
 *
 * * Bits 16-23 contain the major version.
 * * Bits 8-15 contain the minor version.
 * * Bits 0-7 contain the patchlevel.
 *
 * In hexadecimal format, the number 0x010203 represents version 1.2.3.
 */
int cmark_version(void);

// The library version string for runtime checks. Also available as macro CMARK_VERSION_STRING for
// compile time checks.
const char* cmark_version_string(void);

#ifndef CMARK_NO_SHORT_NAMES
    #define NODE_DOCUMENT       CMARK_NODE_DOCUMENT
    #define NODE_BLOCK_QUOTE    CMARK_NODE_BLOCK_QUOTE
    #define NODE_LIST           CMARK_NODE_LIST
    #define NODE_ITEM           CMARK_NODE_ITEM
    #define NODE_CODE_BLOCK     CMARK_NODE_CODE_BLOCK
    #define NODE_HTML_BLOCK     CMARK_NODE_HTML_BLOCK
    #define NODE_CUSTOM_BLOCK   CMARK_NODE_CUSTOM_BLOCK
    #define NODE_PARAGRAPH      CMARK_NODE_PARAGRAPH
    #define NODE_HEADING        CMARK_NODE_HEADING
    #define NODE_HEADER         CMARK_NODE_HEADER
    #define NODE_THEMATIC_BREAK CMARK_NODE_THEMATIC_BREAK
    #define NODE_HRULE          CMARK_NODE_HRULE
    #define NODE_TEXT           CMARK_NODE_TEXT
    #define NODE_SOFTBREAK      CMARK_NODE_SOFTBREAK
    #define NODE_LINEBREAK      CMARK_NODE_LINEBREAK
    #define NODE_CODE           CMARK_NODE_CODE
    #define NODE_HTML_INLINE    CMARK_NODE_HTML_INLINE
    #define NODE_CUSTOM_INLINE  CMARK_NODE_CUSTOM_INLINE
    #define NODE_EMPH           CMARK_NODE_EMPH
    #define NODE_STRONG         CMARK_NODE_STRONG
    #define NODE_LINK           CMARK_NODE_LINK
    #define NODE_IMAGE          CMARK_NODE_IMAGE
    #define BULLET_LIST         CMARK_BULLET_LIST
    #define ORDERED_LIST        CMARK_ORDERED_LIST
    #define PERIOD_DELIM        CMARK_PERIOD_DELIM
    #define PAREN_DELIM         CMARK_PAREN_DELIM
#endif
