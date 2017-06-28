#include "SkimmingNtuples.h"
#include "TLorentzVector.h"
#include <cstdlib>
#include "HistoConfig.h"
#include <iostream>
#include "SVFitObject.h"
#include "SimpleFits/FitSoftware/interface/Logger.h"
 
SkimmingNtuples::SkimmingNtuples(TString Name_, TString id_):
  Selection(Name_,id_)
{
}

SkimmingNtuples::~SkimmingNtuples(){
  for(unsigned int j=0; j<Npassed.size(); j++){
	 Logger(Logger::Info) << "Selection Summary before: "
	 << Npassed.at(j).GetBinContent(1)     << " +/- " << Npassed.at(j).GetBinError(1)     << " after: "
	 << Npassed.at(j).GetBinContent(NCuts+1) << " +/- " << Npassed.at(j).GetBinError(NCuts) << std::endl;
  }
  Logger(Logger::Info) << "complete." << std::endl;
}

void  SkimmingNtuples::Configure(){
  // Setup Cut Values
  for(int i=0; i<NCuts;i++){
    cut.push_back(0);
    value.push_back(0);
    pass.push_back(false);
    if(i==TriggerOk)    cut.at(TriggerOk)=1;
    if(i==ntaus)          cut.at(ntaus)=1;
    if(i==nmuons)      cut.at(nmuons)=1;
    if(i==PrimeVtx)     cut.at(PrimeVtx)=1;
  }
  // Setup cut plots
  TString hlabel;
  TString htitle;
  for(int i=0; i<NCuts; i++){
    title.push_back("");
    distindx.push_back(false);
    dist.push_back(std::vector<float>());
    TString c="_Cut_";c+=i;
    if(i==PrimeVtx){
      title.at(i)="Number of Prime Vertices $(N>$";
      title.at(i)+=cut.at(PrimeVtx);
      title.at(i)+=")";
      htitle=title.at(i);
      htitle.ReplaceAll("$","");
      htitle.ReplaceAll("\\","#");
      hlabel="Number of Prime Vertices";
      Nminus1.push_back(HConfig.GetTH1D(Name+c+"_Nminus1_PrimeVtx_",htitle,11,-0.5,10.5,hlabel,"Events"));
      Nminus0.push_back(HConfig.GetTH1D(Name+c+"_Nminus0_PrimeVtx_",htitle,11,-0.5,10.5,hlabel,"Events"));
    }
    else if(i==TriggerOk){
      title.at(i)="Trigger ";
      hlabel="Trigger ";
      Nminus1.push_back(HConfig.GetTH1D(Name+c+"_Nminus1_TriggerOk_",htitle,2,-0.5,1.5,hlabel,"Events"));
      Nminus0.push_back(HConfig.GetTH1D(Name+c+"_Nminus0_TriggerOk_",htitle,2,-0.5,1.5,hlabel,"Events"));
    }
    else if(i==ntaus){
      title.at(i)="Number of tau leptons ";
      hlabel="Number of tau leptons  ";
      Nminus1.push_back(HConfig.GetTH1D(Name+c+"_Nminus1_ntaus_",htitle,10,-0.5,9.5,hlabel,"Events"));
      Nminus0.push_back(HConfig.GetTH1D(Name+c+"_Nminus0_ntaus_",htitle,10,-0.5,9.5,hlabel,"Events"));
    }
    else if(i==nmuons){
      title.at(i)="Number of muons ";
      hlabel="Number of muons  ";
      Nminus1.push_back(HConfig.GetTH1D(Name+c+"_Nminus1_nmuons_",htitle,10,-0.5,9.5,hlabel,"Events"));
      Nminus0.push_back(HConfig.GetTH1D(Name+c+"_Nminus0_nmuons_",htitle,10,-0.5,9.5,hlabel,"Events"));
    }


  } 
  // Setup NPassed Histogams
  Npassed=HConfig.GetTH1D(Name+"_NPass","Cut Flow",NCuts+1,-1,NCuts,"Number of Accumulative Cuts Passed","Events");
  TauDecayMode=HConfig.GetTH1D(Name+"_TauDecayMode","HPS decay type",11,-0.5,10.5," pType","Events");

  TauPT=HConfig.GetTH1D(Name+"_TauPT","Transverse momentum #tau",50,15,75," P_{T}, GeV","Events");
  MuonPT=HConfig.GetTH1D(Name+"_MuonPT","Transverse momentum  #mu " ,50,15,75," P_{T}, GeV","Events");
  ElePT=HConfig.GetTH1D(Name+"_ElePT","Transverse momentum  e",50,15,75," P_{T}, GeV","Events");
  JetPT=HConfig.GetTH1D(Name+"_JetPT","Transverse momentum jet",50,15,75," P_{T}, GeV","Events");


    Selection::ConfigureHistograms();   //   do not remove
    HConfig.GetHistoInfo(types,CrossSectionandAcceptance,legend,colour);  // do not remove
}

 

void  SkimmingNtuples::Store_ExtraDist(){

  //every new histo should be addedd to Extradist1d vector, just push it back;
Extradist1d.push_back(&TauDecayMode);

 Extradist1d.push_back(&TauPT);
 Extradist1d.push_back(&MuonPT);
 Extradist1d.push_back(&ElePT);
 Extradist1d.push_back(&JetPT);

}

void  SkimmingNtuples::doEvent(){ //  Method called on every event
  unsigned int t;                // sample type, you may manage in your further analysis, if needed
  int id(Ntp->GetMCID());  //read event ID of a sample
  if(!HConfig.GetHisto(Ntp->isData(),id,t)){ Logger(Logger::Error) << "failed to find id" <<std::endl; return;}  //  gives a warining if list of samples in Histo.txt  and SkimSummary.log do not coincide 
  // std::cout<<"------------------ New Event -----------------------"<<std::endl;
  bool PassedTrigger(false);
  int triggerindex;
  std::vector<int> TriggerIndex; 
  std::vector<int>TriggerIndexVector ;
  std::vector<TString>  MatchedTriggerNames;

  //MatchedTriggerNames.push_back("DoubleMediumIsoPFTau");
  //  MatchedTriggerNames.push_back("DoubleMediumCombinedIsoPFTau");
  //  MatchedTriggerNames.push_back("LooseCombinedIsoPFTau");
  MatchedTriggerNames.push_back("IsoPFTau");

  TriggerIndexVector=Ntp->GetVectorTriggers(MatchedTriggerNames);

  //  TriggerIndex=Ntp->GetVectorTriggers("LooseIsoPFTau");
  // TriggerIndex=Ntp->GetVectorTriggers("HLT_IsoMu24");

   for(int itrig = 0; itrig < TriggerIndexVector.size(); itrig++){
     if(Ntp->TriggerAccept(TriggerIndexVector.at(itrig))){  
       //   std::cout<<"  Name  "<< Ntp->TriggerName(TriggerIndexVector.at(itrig)) << "   status   "<< Ntp->TriggerAccept(TriggerIndexVector.at(itrig)) <<std::endl;
       PassedTrigger =Ntp->TriggerAccept(TriggerIndexVector.at(itrig)); }
   }


  // for(unsigned int itr = 0; itr < Ntp->NTriggers(); itr++){
    //    std::cout<<"====  Name  "<< Ntp->TriggerName(itr) << "   status   "<< Ntp->TriggerAccept(itr) <<std::endl;
  //}

  // std::vector<int> TriggerIndex;
  // TriggerIndex=Ntp->GetVectorTriggers("LooseIsoPFTau");


   // for(int itrig = 0; itrig < TriggerIndex.size(); itrig++){

   //   if(   Ntp->GetTriggerIndex((TString)"HLT_IsoMu", TriggerIndex.at(itrig) )   ) {
   //     if(Ntp->TriggerAccept(TriggerIndex.at(itrig))){   
   // 	 std::cout<<" -----------  Name  "<< Ntp->TriggerName(TriggerIndex.at(itrig)) << "   status   "<< Ntp->TriggerAccept(TriggerIndex.at(itrig)) <<std::endl;
   // 	 PassedTrigger =Ntp->TriggerAccept(TriggerIndex.at(itrig)); }
   //   }
   //  //if(Ntp->GetTriggerIndex("LooseIsoPFTau",itrig)) {PassedTrigger = Ntp->TriggerAccept(itrig); triggerindex=itrig; break; }
   // }





  std::vector<int> GoodTausIndex;
  std::vector<int> GoodMuonIndex;
  for(unsigned int iDaugther=0;   iDaugther  <  Ntp->NDaughters() ;iDaugther++ ){  // loop over all daughters in the event
    //    if(Ntp->isLooseGoodTau(iDaugther)) {
      if(Ntp->isMediumGoodTau(iDaugther)) {
      if(Ntp->tauBaselineSelection(iDaugther)){
	GoodTausIndex.push_back(iDaugther);
      }
    }
    if(Ntp->isLooseGoodMuon(iDaugther)){ 
      if(Ntp->muonBaselineSelection(iDaugther)){
      GoodMuonIndex.push_back(iDaugther);}
    }
  }



  // for(int itrig = 0; itrig < TriggerIndex.size(); itrig++){
  //   if(Ntp->GetTriggerIndex((TString)"HLT_IsoMu", TriggerIndex.at(itrig))) {
  //     if(Ntp->TriggerAccept(TriggerIndex.at(itrig))){   PassedTrigger =Ntp->TriggerAccept(TriggerIndex.at(itrig)); }
  //   }
  // }

 

  // Apply Selection
  // two vectors value and pass  are used to apply selection, store value.at(<A cut variable defined in .h as enumerator>) a quantitity you want to use in your selection.
  // Define in the pass.at(<A cut variable defined in .h as enumerator>) a passing condidtion, true/false
  value.at(PrimeVtx)=Ntp->NVtx();
  pass.at(PrimeVtx)=(value.at(PrimeVtx)>=cut.at(PrimeVtx));
  
  value.at(TriggerOk)=PassedTrigger;
  pass.at(TriggerOk)=PassedTrigger;
  
  value.at(ntaus)=GoodTausIndex.size();
  pass.at(ntaus) = (value.at(ntaus) >= cut.at(ntaus));

  value.at(nmuons)=GoodMuonIndex.size();
  pass.at(nmuons) = true;//(value.at(nmuons) >= cut.at(nmuons));

  // Here you can defined different type of weights you want to apply to events. At the moment only PU weight is considered if event is not data
  double wobs=1;
  double w;
  if(!Ntp->isData()){w = Ntp->PUReweight();}
  else{w=1;}



  bool status=AnalysisCuts(t,w,wobs);  // boolean that say whether your event passed critera defined in pass vector. The whole vector must be true for status = true
  ///////////////////////////////////////////////////////////
  // Analyse events  which passed selection
  if(status){
    for(unsigned int itau =0; itau<GoodTausIndex.size(); itau++){
      TauDecayMode.at(t).Fill(Ntp->decayMode(itau),w);
      TauPT.at(t).Fill(Ntp->Daughters_P4(itau).Pt(),w);
    }


    for(unsigned int iDaugther=0;   iDaugther  <  Ntp->NDaughters() ;iDaugther++ ){  // loop over all daughters in the event
      if(Ntp->particleType(iDaugther) == 0){
	MuonPT.at(t).Fill(Ntp->Daughters_P4(iDaugther).Pt(),w);
      }
      if(Ntp->particleType(iDaugther) == 1){
	ElePT.at(t).Fill(Ntp->Daughters_P4(iDaugther).Pt(),w);
      }
    }


     if(Ntp->NJets()!=0)JetPT.at(t).Fill(Ntp->Jet_P4(0).Pt(),w);
  }
}




//  This is a function if you want to do something after the event loop
void  SkimmingNtuples::Finish(){
  Selection::Finish();
}





