%%%
%%% Authors:
%%%   Joerg Wuertz (wuertz@dfki.de)
%%%
%%% Contributors:
%%%   Tobias Mueller (tmueller@ps.uni-sb.de)
%%%
%%% Copyright:
%%%   Joerg Wuertz, 1996
%%%   Tobias Mueller, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%%
%%% Load the application
%%%

\define ALONEDEMO

functor

require
   DemoUrls(image)


prepare
   ImageNames = [DemoUrls.image#'college/title.xbm']

import
   Tk
   TkTools
   Application
   FD
   Search
   OS
   Open
   Compiler
   Browser(browse: Browse)

define
   
   Images = {TkTools.images ImageNames}

   TopWindow
   ControllerLabel
   
   \insert 'College/Schedule.oz'
   
   
   {Controller}
end
