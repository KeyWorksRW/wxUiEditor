// HTML rendering utilities (inline helpers, not main renderer)
// Purpose: Helper functions for HTML output - CR handling, sourcepos attributes
// Key functions: cmark_html_render_cr(), cmark_html_render_sourcepos()
// Note: Main HTML renderer is in html.cpp - this is just helper utilities
// Status: Legacy code
// Dependencies: buffer.hxx, node.hxx

#pragma once

#include "buffer.hxx"

void cmark_html_render_cr(CMarkStringBuffer* html);
