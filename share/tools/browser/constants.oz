%% Programming Systems Lab, University of Saarland,
%% Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5337
%% Author: Konstantin Popov & Co. 
%%  (i.e. all people who make proposals, advices and other rats at all:))
%% Last modified: $Date$ by $Author$
%% Version: $Revision$

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%
%%%
%%%  Local constants;
%%%  This file should be included in a declaration part 
%%% (i.e. local ... \insert 'constants.oz' ... in ... end)
%%%
%%%

%%%
%%% 
%%%  *Real* constants
%%% (i.e. don't touch them at all - they must be different names);
%%% 

%% 
%% INIT value; 
%% 
InitValue       = {NewName}

%%
%% Types of Terms -
%% should be parametric, because Oz has the strong trend to be modified :)) 
%% You have to modify 'terms.oz', 'textWidget.oz' and 'TermsStore.oz' if
%% you change types;
%%

%%
%% group #0: root term (top-level term);
T_RootTerm      = {NewName}

%%
%% group #1: primitive values;
T_Atom          = {NewName}
T_Int           = {NewName}
T_Float         = {NewName}
%%
T_Name          = {NewName}
%%
T_Procedure     = {NewName}
T_Cell          = {NewName}
%%
T_PrimChunk     = {NewName}
T_PrimObject    = {NewName}
T_PrimClass     = {NewName}

%%
%% group #2: compound values;
T_List          = {NewName}
%%
T_Tuple         = {NewName}
%%
T_HashTuple     = {NewName}
%% 
T_Record        = {NewName}
%% these are derived from records;
T_CompChunk     = {NewName}
T_CompObject    = {NewName}
T_CompClass     = {NewName}

%%
%% group #3: variables (but not OFSs);
%% special: if a variable is not yet constrained somehow,
%% we say that this is *something* of the type T_Variable;
%% Of course, such operation is not monotonic, but we need it in browser.
T_Variable      = {NewName}
%%  ... and FD variable; 
T_FDVariable    = {NewName}
%%  ... and Meta variable; 
T_MetaVariable  = {NewName}

%%
%% group #4: What for a bullsh$t ??!
T_Unknown       = {NewName}

%%
%% group #5: specials (not constraint-system dependent);
%% Reference; 
T_Reference     = {NewName}
%% Unshown (sub)term (leaf); 
T_Shrunken      = {NewName}

%%
%% There are actually the following parameters in browser's store 
%% (these are the names of corresponding features in store)": 
%%
StoreTWWidth         = {NewName}
StoreXSize           = {NewName}
StoreXMinSize        = {NewName}
StoreYSize           = {NewName}
StoreYMinSize        = {NewName}
StoreDepth           = {NewName}
StoreWidth           = {NewName}
StoreFillStyle       = {NewName}
StoreArityType       = {NewName}
StoreWidthInc        = {NewName}
StoreDepthInc        = {NewName}
% StoreSmoothScrolling = {NewName}
StoreShowGraph       = {NewName}
StoreShowMinGraph    = {NewName}
StoreSmallNames      = {NewName}
StoreAreVSs          = {NewName}
StoreTWFont          = {NewName}
StoreBufferSize      = {NewName}
StoreWithMenus       = {NewName}
%%
StoreIsWindow        = {NewName}
StoreAreMenus        = {NewName}
%%
StoreBrowserObj      = {NewName}
StoreStreamObj       = {NewName}
StoreOrigWindow      = {NewName}
StoreScreen          = {NewName}
%% 
StoreBreak           = {NewName}
%%
StoreSeqNum          = {NewName}

%%
%% ypes of (record) filling:
Expanded             = {NewName}
Filled               = {NewName}
%% ypes of arity listing:
AtomicArity          = {NewName}
TrueArity            = {NewName}

%%%
%%% 
%%%  (configurable) parameters (i.e. touch them at your own risk :-));
%%% 

%% big enough:)) 
DInfinite       = 1000000

%%
OzHome = {System.get home}

%%
%% Help file;
%%
IHelpFile   = !OzHome # '/lib/browser/help.txt'

%% 
%% window(graphic) parameters; 
%%
%%

%% initial size of a 'browse' text widget (in cols/lines);
ITWWidth    = 60
ITWHeight   = 20

%% initial size of a messages' text widget; 
IMWidth     = 45
IMHeight    = 5

%%
ITitle      = "Oz Browser"
IVTitle     = "Oz Browser: View"
IMTitle     = "Oz Browser: Warnings"
IHTitle     = "Oz Browser: Help"

%%
IITitle     = "Oz Browser"
IMITitle    = "Messages"
IHITitle    = "Help"

%%
IIBitmap    = !OzHome # '/lib/bitmaps/browserIcon.xbm'
IMIBitmap   = !OzHome # '/lib/bitmaps/browserMIcon.xbm'

%% curosr name (see include file X11/cursorfont.h); 
ICursorName    = 'hand2'
ICursorClock   = 'watch'

%%
%% he following two are in pixels (no subwindows are gridded); 
%% hese sizes are used if there is(are) no buttons and/or menus frame(s); 
IXMinSize   = 450
IYMinSize   = 300

%%
IXSize      = 500
IYSize      = 350

%%  ... for messages' window (but it is gridded, so - in chars); 
IMXMinSize  = 40
IMYMinSize  = 5

%%  ... for help window - the fixed size;
IHXSize     = 550
IHYSize     = 400

%% colours on text widget;
IBackGround   = 'white'
IForeGround   = 'black'

%% borders and scrollbars' sizes are in pixels; 
IBigBorder    = 3
ISmallBorder  = 2

%% ext widgets are raised,
%% while menus, buttons and scrollbars frames are sunken; 
ITextRelief   = raised
IFrameRelief  = sunken
IButtonRelief = raised

%% width (or height) of scrollbars (pixels); 
ISWidth       = 15

%%
%% default font for text widgets, 'x' and 'y' resolutions for it; 
ITWFontUnknown = font(name:'*startup*'
		      font:'-*-*-*-*-*-*-*-*-*-*-*-*-*-1'
		      xRes:0 yRes:0)   % these are just some values!
%%
ITWFont1       = font(name:'8x13' font:'8x13' xRes:8 yRes:13)
ITWFont2       = font(name:'14'
		      font:'-*-courier-medium-r-*-*-14-*-*-*-*-*-*-*'
		      xRes:0 yRes:0)
ITWFont3       = font(name:'*any*'
		      font:'-*-*-*-*-*-*-*-*-*-*-*-*-*-1'
		      xRes:0 yRes:0)   % these are just some values!
IKnownMiscFonts = [font(name:'10x20' font:'10x20' xRes:10 yRes:20)
		   font(name:'9x15' font:'9x15' xRes:9 yRes:15)
		   font(name:'9x15bold' font:'9x15bold' xRes:9 yRes:15)
		   font(name:'8x13' font:'8x13' xRes:8 yRes:13)
		   font(name:'8x13bold' font:'8x13bold' xRes:8 yRes:13)
		   font(name:'7x13' font:'7x13' xRes:7 yRes:13)
		   font(name:'7x13bold' font:'7x13bold' xRes:7 yRes:13)
		   font(name:'6x13' font:'6x13' xRes:6 yRes:13)
		   font(name:'6x13bold' font:'6x13bold' xRes:6 yRes:13)
		   font(name:'6x12' font:'6x12' xRes:6 yRes:12)
		   font(name:'6x10' font:'6x10' xRes:6 yRes:10)
		   font(name:'any'
			font:'-*-*-*-*-*-*-*-*-*-*-*-*-*-1'
			xRes:0 yRes:0)]

%%
%% Note: actually, precise dimentions of the courier fonts are not
%% known (and even more, they depend on many other factors!). So, we
%% just left them unspecified (i.e. == 0).
%% 
IKnownCourFonts = [font(name:'24'
			font:'-*-courier-medium-r-*-*-24-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'24bold'
			font:'-*-courier-bold-r-*-*-24-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'18'
			font:'-*-courier-medium-r-*-*-18-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'18bold'
			font:'-*-courier-bold-r-*-*-18-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'14'
			font:'-*-courier-medium-r-*-*-14-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'14bold'
			font:'-*-courier-bold-r-*-*-14-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'12'
			font:'-*-courier-medium-r-*-*-12-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'12bold'
			font:'-*-courier-bold-r-*-*-12-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'10'
			font:'-*-courier-medium-r-*-*-10-*-*-*-*-*-*-*'
			xRes:0 yRes:0)
		   font(name:'10bold'
			font:'-*-courier-bold-r-*-*-10-*-*-*-*-*-*-*'
			xRes:0 yRes:0)]

%%
%% (external) pads (pixels); 
IPad          = 1
IButtonPad    = 1
ITWPad        = 3

%% width of a button (in cm); 
IButtonWidth  = 10

%%
IBFont1       = '-*-lucida-bold-r-*-sans-12-*-*-*-*-*-*-*'
IBFont2       = '-*-courier-bold-r-*-*-12-*-*-*-*-*-*-*'
IBFont3       = '-*-*-*-r-*-*-12-*-*-*-*-*-*-1'

%%  "any" font: if the specified one is not found;
IReservedFont = '-*-*-*-*-*-*-*-*-*-*-*-*-*-1'

%%
%% offset (<x,y> from the upper left conner) for a message window; 
IMWXOffset    = 50
IMWYOffset    = 50

%%%
%%% 
%%%  defaults section (for "store") (i.e. it should be changeable);
%%% 

%%
IDepth            = 15
IWidth            = 50
IFillStyle        = Expanded
IArityType        = AtomicArity
ISmallNames       = !True
IAreVSs           = !False
IDepthInc         = 1
IWidthInc         = 1
% ISmoothScrolling  = !False
IShowGraph        = !False
IShowMinGraph     = !False  
%% ... only one of the previous two should be toggled one;
IBufferSize       = 15
%% would the menus frame come at start? 
IWithMenus        = !True 

%%%
%%%
%%%  internal parameters
%%% (i.e. you probably have to know what they actually mean);
%%%

%%
%% How much elements in a term store list should be per one failure
%% during searching;
TermsStoreGCRatio  = 5
%%  ... The base for comparision, i.e. it looks like
%% case Fails * TermsStoreGCRatio > Size + TermsStoreGCBase
%% hen {DO_GC}
%% else true
%% end
TermsStoreGCBase    = 100
%% How much a real size of a 'ResizableArray' can be bigger
%% han 'visible';
RArrayRRatio       = 10

%%%
%%% 
%%%  Various definitions for debugging
%%% (turning them on will end up with many debug Show"s);
%%%

/*
\define    DEBUG_BO
%\undef     DEBUG_BO
\define    DEBUG_WM
%\undef     DEBUG_WM
\define    DEBUG_MO
%\undef     DEBUG_MO
\define    DEBUG_CO
%\undef     DEBUG_CO
\define    DEBUG_RM
%\undef     DEBUG_RM
\define    DEBUG_TO
%\undef     DEBUG_TO
\define    DEBUG_TI
%\undef     DEBUG_TI
*/

% /*
%\define    DEBUG_BO
\undef     DEBUG_BO
%\define    DEBUG_WM
\undef     DEBUG_WM
%\define    DEBUG_MO
\undef     DEBUG_MO
%\define    DEBUG_CO
\undef     DEBUG_CO
%\define    DEBUG_RM
\undef     DEBUG_RM
%\define    DEBUG_TO
\undef     DEBUG_TO
%\define    DEBUG_TI
\undef     DEBUG_TI
% */

%%%
%%%
%%%  Work-Arounds;
%%%

%% none at the moment :-)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%
%%%
%%%      Don't change anything below this line!
%%%
%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%
%%%  ... it means that otherwise something will definitely go wrong !!!
%%%

%% spacing between atoms, etc. in output:
%% size of blank, hash, colon, braces and '='; 
DSpace          = 1
DDSpace         = 2
DTSpace         = 3
DQSpace         = 4

%% spacing to subterms of a {record, tuple}; 
DOffset         = 3

%% max approx; 
DReference      = 2
%% 'R?' 

%%
%% Note that glues, etc. below *must be atoms*. Otherwise, the
%% CoreTerms.delimiterLEQ must be updated;
%%
%% glues; should be of the same length (DSpace);
DSpaceGlue      = ' '		% DSpace;
DHashGlue       = "#"		% note: it must be a string!

%% symbols;
DLRBraceS       = '('
DRRBraceS       = ')'
DLSBraceS       = '['
DRSBraceS       = ']'
DLABraceS       = '<'
DRABraceS       = '>'
DEqualS         = '='
DColonS         = ':'
DLCBraceS       = '{'
DRCBraceS       = '}'

%%
DNameUnshown    = ',,,'		% DTSpace;
DOpenFS         = '...'		% DTSpace;
DDBar           = '||'          % DDSpace;
DUnderscore     = '_'		% DSpace;
DUnshownPFs     = '?'		% DSpace

%% char values; 
CNameDelimiter  = &:
BQuote          = &`
CharDot         = &.
CharSpace       = " ".1		% so and only so!
/* ` oz emacs modus problems; */
%%

%%
DRootGroup      = 0#1

%%
%% In general, there are five block in each of contemporary compound
%% term's representation:
%%
%% The leading group number;
DLeadingBlock   = 0
DMainBlock      = 1
%% the following block can contain a ',,,' group;
DCommasBlock    = 2
%% ... and this one - some ellipses, currently, there are just
%% ellipses for records and a '|| _' sequence for lists;
DSpecialBlock   = 3
DTailBlock      = 4

%%
%% in a leading block, there can be the following groups:
DLabelGroup     = 1
DLRBraceGroup   = 2
%% or, for lists:
DLSBraceGroup   = 1

%% within a commas block, there can be an ellipses group:
DCommasGroup    = 1
%% within a special block, there can be following groups:
DEllipsesGroup  = 1
%% or, 
DDBarGroup      = 1
DLTGroup        = 2

%%
%% tail block can either absent, of contain a brace (round or square):
DBraceGroup     = 1
