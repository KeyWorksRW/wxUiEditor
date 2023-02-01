//////////////////////////////////////////////////////////////////////////
// Purpose:   Common Book utilities
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Node;
class Code;

// Walks up the parent tree until it finds a Book and returns whether or not the book is
// supposed to display images. This function will handle wxTreeBook with any depth of sub
// pages.
bool isBookDisplayImages(Node* node);

// This will walk through all of a book's pages to see if any of them have an image. This
// will handle wxTreebook with an unlimited number of nested sub pages.
bool isBookHasImage(Node* node);

void AddBookImageList(Node* node_book, wxObject* widget);

// Creates bundle_list and adds all bundles to it. In C++ the code will be in brackets.
void BookCtorAddImagelist(Code& code);

void AddTreebookSubImages(Node* node, wxBookCtrlBase::Images& bundle_list);

void AddTreebookImageCode(tt_string& code, Node* child_node, size_t& image_index);

int GetTreebookImageIndex(Node* node);
