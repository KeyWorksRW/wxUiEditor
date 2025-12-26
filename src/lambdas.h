/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for formatting and storage of lamda events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
