// -*- C++ -*-
// Usage:
// > root -b -q doAll.C

#include <iostream>
#include <vector>
#include <numeric>

// ROOT
#include "TBenchmark.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TROOT.h"
#include "TTreeCache.h"
#include "TCanvas.h"
#include "TCut.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TPaveStats.h"

// CMS3
#include "CMS3.cc"

using namespace std;
using namespace tas;

int ScanChain(TChain* chain, bool fast = true, int nEvents = -1, string skimFilePrefix = "test") {

  // Benchmark
  TBenchmark *bmark = new TBenchmark();
  bmark->Start("benchmark");

  // Creating Histograms
  TDirectory *rootdir = gDirectory->GetDirectory("Rint:");
  TCanvas *c_1 = new TCanvas("c_1", "Plots", 200, 200, 800, 700);
  c_1->Divide(2,2);

  TH1F *ht = new TH1F("ht", "ht Histogram", 100,0,3000);
  TH1F *met_pt = new TH1F("met_pt", "met_pt Histogram", 100, 0, 2500);
  TH1F *mt2 = new TH1F("mt2", "mt2 Histogram", 100, 0, 1500);
  TH1F *nJet30 = new TH1F("nJet30", "nJet30 Histogram", 11, 0, 11);
  
  TH1F *ht_cut = new TH1F("ht_cut", "ht Histogram", 100,0,3000);
  TH1F *met_pt_cut = new TH1F("met_pt_cut", "met_pt Histogram", 100, 0, 2500);
  TH1F *mt2_cut = new TH1F("mt2_cut", "mt2 Histogram", 100, 0, 1500);
  TH1F *nJet30_cut = new TH1F("nJet30_cut", "nJet30 Histogram", 11, 0, 11);
  //ht->SetDirectory(rootdir);

  // Loop over events to Analyze
  unsigned int nEventsTotal = 0;
  unsigned int nEventsChain = chain->GetEntries();
  if (nEvents >= 0) nEventsChain = nEvents;
  TObjArray *listOfFiles = chain->GetListOfFiles();
  TIter fileIter(listOfFiles);
  TFile *currentFile = 0;

  // File Loop
  while ( (currentFile = (TFile*)fileIter.Next()) ) {

    // Get File Content
    TFile file(currentFile->GetTitle());
    TTree *tree = (TTree*)file.Get("mt2");
    if (fast) TTreeCache::SetLearnEntries(10);
    if (fast) tree->SetCacheSize(128*1024*1024);
    cms3.Init(tree);

    // Loop over Events in current file
    if (nEventsTotal >= nEventsChain) continue;
    unsigned int nEventsTree = tree->GetEntriesFast();
    for (unsigned int event = 0; event < nEventsTree; ++event) {

      // Get Event Content
      if (nEventsTotal >= nEventsChain) continue;
      if (fast) tree->LoadTree(event);
      cms3.GetEntry(event);
      ++nEventsTotal;

      // Progress
      CMS3::progress( nEventsTotal, nEventsChain );

      /********************************************************************************************/
      //Alalysis Code

      ht->Fill(cms3.ht());
      met_pt->Fill(cms3.met_pt());
      double mt2_tmp = cms3.mt2();
      if(!std::isnan(mt2_tmp)) mt2->Fill(mt2_tmp);
      nJet30->Fill(cms3.nJet30());

      // Applying cuts to data
      if((cms3.nMuons10()==0 && cms3.nElectrons10()==0 && cms3.nPFLep5LowMT() == 0 && cms3.nPFHad10LowMT() == 0) && // lepveto cut
	 (cms3.HLT_PFHT900() || cms3.HLT_PFJet450() || cms3.HLT_PFHT300_PFMET110() || cms3.HLT_PFMET120_PFMHT120() || cms3.HLT_PFMETNoMu120_PFMHTNoMu120()) && // trigs cut
	 ((cms3.nVert() > 0) && (cms3.nJet30() > 1) && (cms3.deltaPhiMin() > 0.3) && (cms3.diffMetMht()/cms3.met_pt() < 0.5) && (cms3.mt2() > 200) 
	  && ( (cms3.ht() > 250 && cms3.met_pt() > 250) || (cms3.ht() > 1000 && cms3.met_pt() > 30) )) && // srbase cut
	 (cms3.Flag_eeBadScFilter() && cms3.Flag_HBHENoiseFilter() && cms3.Flag_HBHENoiseIsoFilter() && cms3.Flag_globalTightHalo2016Filter() 
	  && cms3.Flag_badMuonFilter() && cms3.Flag_goodVertices() && cms3.Flag_EcalDeadCellTriggerPrimitiveFilter() && cms3.Flag_badChargedHadronFilter() 
	  && cms3.nJet30FailId() == 0 && cms3.met_miniaodPt() / cms3.met_caloPt() < 5.0 && cms3.nJet200MuFrac50DphiMet() == 0)) { // filters cut
	      // Fill the following Histograms
	      ht_cut->Fill(cms3.ht());
	      met_pt_cut->Fill(cms3.met_pt());
	      if(!std::isnan(mt2_tmp)) mt2_cut->Fill(mt2_tmp);
	      nJet30_cut->Fill(cms3.nJet30());
      }
    }   
      // Normalizing Background
      Double_t ht_norm = (ht_cut->Integral())/(ht->Integral());
      ht->Scale(ht_norm);
      Double_t met_pt_norm = (met_pt_cut->Integral())/(met_pt->Integral());
      met_pt->Scale(met_pt_norm);
      Double_t mt2_norm = (mt2_cut->Integral())/(mt2->Integral());
      mt2->Scale(mt2_norm);
      Double_t nJet30_norm = (nJet30_cut->Integral())/(nJet30->Integral());
      nJet30->Scale(nJet30_norm);

      // Changing appearance
      ht->SetLineColor(kBlue-7);
      met_pt->SetLineColor(kBlue-7);
      mt2->SetLineColor(kBlue-7);
      nJet30->SetLineColor(kBlue-7);

      ht_cut->SetFillColor(kRed-7);
      met_pt_cut->SetFillColor(kRed-7);
      mt2_cut->SetFillColor(kRed-7);
      nJet30_cut->SetFillColor(kRed-7);
  
    // Clean Up
    delete tree;
    file.Close();
  }
  if (nEventsChain != nEventsTotal) {
    cout << Form( "ERROR: number of events from files (%d) is not equal to total number of events (%d)", nEventsChain, nEventsTotal ) << endl;
  }
  
  // gStyle->SetOptStat(0);
  
   // Setting legend
  TLegend *legend1 = new TLegend(0.68, 0.8, 0.98, 0.95);
  legend1->SetHeader("Legend","C"); // option "C" allows to center the header
  legend1->AddEntry(ht,"Signal Plus Background", "f");
  legend1->AddEntry(ht_cut,"Signal", "f");

  TLegend *legend2 = new TLegend(0.68, 0.8, 0.98, 0.95);
  legend2->SetHeader("Legend","C"); // option "C" allows to center the header
  legend2->AddEntry(met_pt,"Signal Plus Background","f");
  legend2->AddEntry(met_pt_cut,"Signal","f");

  TLegend *legend3 = new TLegend(0.68, 0.8, 0.98, 0.95);
  legend3->SetHeader("Legend","C"); // option "C" allows to center the header                                                                                                                                   
  legend3->AddEntry(mt2,"Signal Plus Background","f");
  legend3->AddEntry(mt2_cut,"Signal","f");

  TLegend *legend4 = new TLegend(0.68, 0.8, 0.98, 0.95);
  legend4->SetHeader("Legend","C"); // option "C" allows to center the header                                                                                                                                   
  legend4->AddEntry(nJet30,"Signal Plus Background","f");
  legend4->AddEntry(nJet30_cut,"Signal","f");

  // Drawing Histograms
  c_1->cd(1); // Go to panel 1
  ht_cut->Draw("hist");  
  gPad->Update();
  TPaveStats *ps = (TPaveStats*)ht_cut->FindObject("stats");
  ps->SetY1NDC(0.62); ps->SetY2NDC(0.78); // Move stats panel
  
  ht->Draw("hist sames");
  gPad->Update();
  ps = (TPaveStats*)ht->FindObject("stats");
  ps->SetY1NDC(0.45); ps->SetY2NDC(0.60);
  legend1->Draw("same");

  c_1->cd(2); // Change to panel 2
  met_pt_cut->Draw("hist");
  gPad->Update();
  ps = (TPaveStats*)met_pt_cut->FindObject("stats");
  ps->SetY1NDC(0.62); ps->SetY2NDC(0.78);
  
  met_pt->Draw("hist sames");
  gPad->Update();
  ps = (TPaveStats*)met_pt->FindObject("stats");
  ps->SetY1NDC(0.45); ps->SetY2NDC(0.60);
  legend2->Draw("same");

  c_1->cd(3); // Change to panel 3
  mt2_cut->Draw("hist");
  gPad->Update();
  ps = (TPaveStats*)mt2_cut->FindObject("stats");
  ps->SetY1NDC(0.62); ps->SetY2NDC(0.78);
  
  mt2->Draw("hist sames");  legend3->Draw("same");
  gPad->Update();
  ps = (TPaveStats*)mt2->FindObject("stats");
  ps->SetY1NDC(0.45); ps->SetY2NDC(0.60);
  legend3->Draw("same");

  c_1->cd(4); // Change to panel 4
  nJet30_cut->Draw("hist");
  gPad->Update();
  ps = (TPaveStats*)nJet30_cut->FindObject("stats");
  ps->SetY1NDC(0.62); ps->SetY2NDC(0.78);

  nJet30->Draw("hist sames");
  gPad->Update();
  ps = (TPaveStats*)nJet30->FindObject("stats");
  ps->SetY1NDC(0.45); ps->SetY2NDC(0.60);
  legend4->Draw("same");   

  // return
  bmark->Stop("benchmark");
  cout << endl;
  cout << nEventsTotal << " Events Processed" << endl;
  cout << "------------------------------" << endl;
  cout << "CPU  Time: " << Form( "%.01f", bmark->GetCpuTime("benchmark")  ) << endl;
  cout << "Real Time: " << Form( "%.01f", bmark->GetRealTime("benchmark") ) << endl;
  cout << endl;
  delete bmark;
  return 0;
}
