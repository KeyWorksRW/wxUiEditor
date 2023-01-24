/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for formatting and storage of lamda events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// Replaces @@ with \n
inline void ExpandLambda(ttlib::cstr& lambda)
{
    lambda.LeftTrim();
    if (lambda.starts_with("@@"))
    {
        lambda.erase(0, 2);
    }
    lambda.Replace("@@", "\n", tt::REPLACE::all);
    lambda.RightTrim();
}
