%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $
import
   System(show)
   GTK at 'x-oz://system/GTK.ozf'
define
   {Wait GTK}
   
   %% Create Toplevel window class
   class MyToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.wINDOW_TOPLEVEL)
      end
      meth connectEvents
	 GTK.window, signalConnect('destroy' destroyEvent _)
	 GTK.window, setBorderWidth(10)
      end
      meth destroyEvent(Event)
	 {System.show 'DesroyEvent occured'}
      end
   end

   %% Create Button class
   class MyButton from GTK.button
      meth new
	 GTK.button, newWithLabel("Hello, GTK!")
      end
      meth connectEvents
	 GTK.button, signalConnect('clicked' clickedEvent _)
      end
      meth clickedEvent(Event)
	 {System.show 'ClickedEvent occured'}
      end
   end

   Toplevel = {New MyToplevel new}
   Button   = {New MyButton new}

   %% Make Butten child of Toplevel
   {Toplevel add(Button)}
   %% Make it all visible
   {Toplevel showAll}
end