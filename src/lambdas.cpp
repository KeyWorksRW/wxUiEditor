/////////////////////////////////////////////////////////////////////////////
// Purpose:   Functions for formatting and storage of lambda events
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

/*

    While this is obviously a very small module, having the code to expand and compress lambda coding in one place will make
    it easier to change if needed.

*/

#include "lambdas.h"

void ExpandLambda(ttlib::cstr& lambda)
{
    lambda.LeftTrim();
    if (lambda.starts_with("@@"))
    {
        lambda.erase(0, 2);
    }
    lambda.Replace("@@", "\n", tt::REPLACE::all);
    lambda.RightTrim();
}

void CompressLambda(ttlib::cstr& lambda)
{
    lambda.insert(0, "@@");
    lambda.Replace("\r", "", tt::REPLACE::all);  // Remove Windows EOL
    lambda.Replace("\n", "@@", tt::REPLACE::all);
    lambda.RightTrim();
}
