////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2011 Bryce Lelbach
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include "stdio.h"

int main()
{
  char tmp[32];
  sprintf(tmp, "%02d%02d%02d", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
  std::cout << tmp;
}

