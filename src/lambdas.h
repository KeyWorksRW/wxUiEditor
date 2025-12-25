/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for formatting and storage of lamda events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wxue_namespace/wxue_string.h"  // wxue::string

// Replaces @@ with \n
inline void ExpandLambda(tt_string& lambda)
{
    lambda.LeftTrim();
    if (lambda.starts_with("@@"))
    {
        lambda.erase(0, 2);
    }
    lambda.Replace("@@", "\n", tt::REPLACE::all);
    lambda.RightTrim();
}

// Replaces @@ with \n (wxue::string version)
inline void ExpandLambda(wxue::string& lambda)
{
    lambda.LeftTrim();
    if (lambda.starts_with("@@"))
    {
        lambda.erase(0, 2);
    }
    lambda.Replace("@@", "\n", wxue::REPLACE::all);
    lambda.RightTrim();
}
