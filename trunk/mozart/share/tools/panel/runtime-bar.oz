%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%


local
   BackColor    = white
   OutlineColor = black
   Width        = 240
   Height       = 20
   Gap          = 2
   Border       = 2
   Y0           = Gap + 1
   Y1           = Height
   Home         = ~10
in

   class RuntimeBar
      from Tk.canvas
      prop final
	 
      feat
	 RunTag
	 GcTag
	 CopyTag
	 PropTag
	 LoadTag
      attr
	 Saved: ZeroTime
	 Clear: ZeroTime

      meth init(parent:P)
	 RuntimeBar,tkInit(parent:             P
			   width:              Width
			   relief:             groove
			   highlightthickness: 0
			   bd:                 Border
			   bg:                 BackColor
			   height:             Height)
	 ThisRunTag  = {New Tk.canvasTag tkInit(parent:self)}
	 ThisGcTag   = {New Tk.canvasTag tkInit(parent:self)}
	 ThisCopyTag = {New Tk.canvasTag tkInit(parent:self)}
	 ThisPropTag = {New Tk.canvasTag tkInit(parent:self)}
	 ThisLoadTag = {New Tk.canvasTag tkInit(parent:self)}
      in
	 self.RunTag  = ThisRunTag
	 self.GcTag   = ThisGcTag
	 self.CopyTag = ThisCopyTag
	 self.PropTag = ThisPropTag
	 self.LoadTag = ThisLoadTag
	 RuntimeBar,tk(crea rectangle Home Y0 Home Y1
		       fill:    TimeColors.run
		       stipple: TimeStipple.run
		       tags:    ThisRunTag)
	 RuntimeBar,tk(crea rectangle Home Y0 Home Y1
		       fill:    TimeColors.gc
		       stipple: TimeStipple.gc
		       tags:    ThisGcTag)
	 RuntimeBar,tk(crea rectangle Home Y0 Home Y1
		       fill:    TimeColors.copy
		       stipple: TimeStipple.copy
		       tags:    ThisCopyTag)
	 RuntimeBar,tk(crea rectangle Home Y0 Home Y1
		       fill:    TimeColors.'prop'
		       stipple: TimeStipple.'prop'
		       tags:    ThisPropTag)
	 RuntimeBar,tk(crea rectangle Home Y0 Home Y1
		       fill:    TimeColors.load
		       stipple: TimeStipple.load
		       tags:    ThisLoadTag)
      end

      meth clear
	 Clear <- @Saved
	 Saved <- ZeroTime
	 RuntimeBar, displayZero
      end

      meth displayZero	       
	 RuntimeBar,tk(coords self.RunTag  Home Y0 Home Y1)
	 RuntimeBar,tk(coords self.GcTag   Home Y0 Home Y1)
	 RuntimeBar,tk(coords self.CopyTag Home Y0 Home Y1)
	 RuntimeBar,tk(coords self.PropTag Home Y0 Home Y1)
	 RuntimeBar,tk(coords self.LoadTag Home Y0 Home Y1)
      end
      
      meth display(T)
	 case T==@Saved then skip else
	    C           = @Clear
	    GcTime      = T.gc   - C.gc
	    CopyTime    = T.copy - C.copy
	    PropTime    = T.propagate - C.propagate
	    RunTime     = T.user - C.user
	    LoadTime    = T.load - C.load
	 in
	    case RunTime==0 then
	       RuntimeBar,displayZero
	    else
	       GcZero    = case GcTime==0   then 0 else 1 end
	       CopyZero  = case CopyTime==0 then 0 else 1 end
	       PropZero  = case PropTime==0 then 0 else 1 end
	       LoadZero  = case LoadTime==0 then 0 else 1 end
	       HalfTime  = RunTime div 2
	       ThisWidth = Width -
			   (GcZero + CopyZero + PropZero + LoadZero + 1) * Gap
	       GcWidth   = (GcTime   * ThisWidth + HalfTime) div RunTime
	       CopyWidth = (CopyTime * ThisWidth + HalfTime) div RunTime
	       PropWidth = (PropTime * ThisWidth + HalfTime) div RunTime
	       LoadWidth = (LoadTime * ThisWidth + HalfTime) div RunTime
	       LoadEnd   = Width
	       LoadStart = LoadEnd - LoadWidth
	       PropEnd   = LoadStart - LoadZero * Gap
	       PropStart = PropEnd - PropWidth
	       CopyEnd   = PropStart - PropZero * Gap
	       CopyStart = CopyEnd - CopyWidth
	       GcEnd     = CopyStart - CopyZero * Gap
	       GcStart   = GcEnd   - GcWidth
	       RunEnd    = GcStart   - GcZero * Gap
	       RunStart  = Gap + 1
	    in
	       Saved <- T
	       RuntimeBar,tk(coords self.RunTag  RunStart  Y0 RunEnd  Y1)
	       RuntimeBar,case GcTime==0 then
			     tk(coords self.GcTag  Home Y0 Home Y1)
			  else
			     tk(coords self.GcTag   GcStart   Y0 GcEnd   Y1)
			  end
	       RuntimeBar,case CopyTime==0 then
			     tk(coords self.CopyTag Home Y0 Home Y1)
			  else
			     tk(coords self.CopyTag CopyStart Y0 CopyEnd Y1)
			  end
	       RuntimeBar,case PropTime==0 then
			     tk(coords self.PropTag Home Y0 Home Y1)
			  else
			     tk(coords self.PropTag PropStart Y0 PropEnd Y1)
			  end
	       RuntimeBar,case LoadTime==0 then
			     tk(coords self.LoadTag Home Y0 Home Y1)
			  else
			     tk(coords self.LoadTag LoadStart Y0 LoadEnd Y1)
			  end
	    end
	 end
      end
   end

end
