/*
 *  Main authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *
 *  Contributing authors:
 *
 *
 *  Copyright:
 *     Alejandro Arbelaez, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

					
   %% Miscellaneous
   Inf = {Int_inf}
   Sup = {Int_sup}
   fun{Decl}
      {IntVar Inf#Sup}
   end


   %%Watching domains 
   fun{WatchDomain P}
      proc{$ D1 D2 B} BTmp in
	 BTmp = {IntVar 0#1}
	 {Wait D2}
	 {P D1 BTmp D2}
	 {Wait BTmp}
	 if BTmp == 1 then B = true else B = false end
      end
   end
   IntWatchMin = {WatchDomain Int_watch_min}
   IntWatchMax = {WatchDomain Int_watch_max}
   IntWatchSize = {WatchDomain Int_watch_size}

   Watch = watch(min:IntWatchMin max:IntWatchMax size:IntWatchSize)

   %%Reflect

   Reflect.nextLarger    = Int_nextLarger
   Reflect.nextSmaller   = Int_nextSmaller
   Reflect.domList       = Int_domlist
   Reflect.dom           = Int_dom
   %%Generic propagators

   proc{SumCN Iv Dvv A D}
      {Int_sumCN Iv Dvv Rt.A D}
   end

   proc{Sum Dv A D}
%      {GFD.linear Dv A D Cl.val}
      {Linear Dv Rt.A D Cl.val}
   end
   
   proc{SumC Iv Dv A D}
      {Linear2 Iv Dv Rt.A D Cl.val}
   end

   proc{SumD Dv A D}
      {Linear Dv Rt.A D Cl.dom}
   end

   proc{SumCD Iv Dv A D}
      {Linear Iv Dv Rt.A D Cl.dom}
   end


   proc{SumAC Iv Dv A D SP} Tmp in
      Tmp = {Decl}
      {SumC Iv Dv A Tmp}
      {Abs Tmp D Cl.val}
   end

   proc{SumACN Iv Dv A D SP} Tmp in
      Tmp = {Decl}
      {SumCN Iv Dv A Tmp}
      {Abs Tmp D Cl.val}
   end

   %%Reified Propagators
   %%FD.reified.sumC +Iv *Dv +A *D1 D2}
   proc{SumR Dv A D1 D2}
      {LinearR Dv Rt.A D1 D2 Cl.val}
   end

   proc{SumCR Iv Dv A D1 D2}
      {LinearCR Iv Dv Rt.A D1 D2 Cl.val}
   end
   Reified.sum = SumR
   Reified.sumC = SumCR

   %%Symbolic propagators
   
   proc{AtLeast D Dv I}
      {Count Dv D Rt.'=<:' I Cl.val}
   end
   
   proc{AtMost D Dv I}
      {Count Dv D Rt.'>=:' I Cl.val}
   end
   
   proc{Exactly D Dv I}
      {Count Dv D Rt.'=:' I Cl.val}
   end

   
   proc{DistinctOffset Dv Iv}
      {Distinct2 Iv Dv Cl.val}
   end

%   proc{Distinct Dv Iv}
%      {Distinct Iv Dv Cl.val}
%   end

   %%Miscellaneous propagators
   Disjoint = Int_disjoint
   DisjointC = _

   proc{Plus D1 D2 D3}
      {Sum [D1 D2] '=:' D3}
   end

   proc{PlusD D1 D2 D3}
      {SumD [D1 D2] '=:' D3}
   end

   proc{Minus D1 D2 D3}
      {Plus D2 D3 D1}
   end

   proc{MinusD D1 D2 D3}
      {PlusD D2 D3 D1}
   end

   proc{Less D1 D2}
      {IntRel D1 Rt.'<:' D2 Cl.val}
   end

   proc{LessEq D1 D2}
      {IntRel D1 Rt.'=<:' D2 Cl.val}
   end

   proc{Greater D1 D2}
      {IntRel D1 Rt.'>:' D2 Cl.val}
   end

   proc{GreaterEq D1 D2}
      {IntRel D1 Rt.'>=:' D2 Cl.val}
   end

   %%may be this proc have to change something after we have all gecode propagators implemented
   proc{Times D1 D2 D3}
      {Mult D1 D2 D3}
   end

   proc{TimesD D1 D2 D3}
      {System.show 'Unsoported propagator because gecode just support value consistence in mult propagator'}
   end

   proc{DivI D1 I D3}
      {Wait I}
      {Mult D3 D1 I}
   end

   %%Assigning Values

   %%Reified Propagators
%%   proc{IntR Spec D1 D2}
%%      {Int_reified_int Spec D1 D2}
%%   end
%%   Reified.int = IntR

   %%Pending ....
   Reified.int = _ 


   %% 0/1 Propagators
   proc{Conj D1 D2 D3}
      {Bool_Gand D1 D2 D3 Cl.val}
   end

   proc{Disj D1 D2 D3}
      {Bool_Gor D1 D2 D3 Cl.val}
   end

   proc{Xor D1 D2 D3}
      {Bool_Gxor D1 D2 D3 Cl.val}
   end

   proc{Not D1 D2}
      {Bool_Gnot D1 D2 Cl.val}
   end

   proc{Imp D1 D2 D3}
      {Bool_Gimp D1 D2 D3 Cl.val}
   end

   proc{Equi D1 D2 D3}
      {Bool_Geqv D1 D2 D3 Cl.val}
   end

   
   
%%end