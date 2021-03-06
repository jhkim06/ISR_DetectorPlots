#ifndef Hists_H
#define Hists_H

#include <TString.h>
#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>
#include <THStack.h>
#include <iostream>

#include <RooDataHist.h>
#include <RooRealVar.h>
#include <RooHistPdf.h>
#include <RooAddPdf.h>
#include <RooFitResult.h>

#include "TUnfoldBinning.h"
#include "TUnfoldDensity.h"

using namespace RooFit;

class Hists
{
    //
    //
private:

    TFile* inFile;
    TH1* hData;
    TH1* hMCSig;
    TH1* hMCBkgs;

    TH1* hFakeEstimation;

    std::map<TString, TString> bkgMap;

    // Bin definitions
    TUnfoldBinning* binDef = NULL;
    std::vector<double> massBinEdges;

    std::vector<TString> bkgNames; // Backgroud Name
    std::vector<TString> bkgTypes; // Backgroud Type

    std::map<TString, int> bkgColors;

    TString histDir;
    TString var;
    TString channel;

    TString channel_postfixMC;
    TString channel_postfixData;

    bool isEstimation;

public:
    Hists(TString filePath, TString histDir_, TString channel_ = "muon", TString var_ = "Mass", bool isEstimation_ = true)
    {
        cout << "Initialize Hists class..." << endl;
        cout << "File: " << filePath << endl;
        inFile = new TFile(filePath, "READ");

        histDir = histDir_;
        cout << "Hist directory: " << histDir << endl;

        var = var_;
        cout << "Variable: " << var << endl;

        channel = channel_;

        // FIXME Update for electron
        if(channel == "Muon")
        {
            channel_postfixMC = "MuMu";
            channel_postfixData = "DoubleMuon";
        }
        else
        {
            channel_postfixMC = "EE";
            //channel_postfixData = "DoubleEG";
            channel_postfixData = "EGamma";
        }

        // FIXME Update for pt
        TString binPath = histDir + "/" + var + "/" + "Rec_Mass";
        if(var=="Pt")
        {
            binPath = histDir + "/" + var + "/" + "Rec_Pt";
        }

        binDef = (TUnfoldBinning*)inFile->Get(binPath);

        if(var=="Pt") setMassBindEdges();

        // Fill colors for backgrounds
        bkgColors["WJets"] = kViolet+1;
        bkgColors["EWK"] = kYellow+2;
        bkgColors["Top"] = kBlue;
        bkgColors["Fake"] = kViolet+2;

        // No sigle Top samples
        bkgMap = {{"WJet", "Fake"},{"QCD","Fake"}, {"WW_pythia", "EWK"}, {"WZ_pythia", "EWK"}, {"ZZ_pythia", "EWK"}, {"DYJets10to50ToTauTau","EWK"}, {"DYJetsToTauTau","EWK"}, 
                 {"TTLL_powheg", "Top"}};
        
        bkgNames ={"WJet", "QCD", "WW_pythia", "WZ_pythia", "ZZ_pythia", "DYJets10to50ToTauTau", "DYJetsToTauTau", "TTLL_powheg"};
        
        //bkgMap = {{"WJet", "Fake"},{"QCD","Fake"}, {"WW_pythia", "EWK"}, {"WZ_pythia", "EWK"}, {"ZZ_pythia", "EWK"}, {"DYJets10to50ToTauTau","EWK"}, {"DYJetsToTauTau","EWK"}, 
        //         {"TTLL_powheg", "Top"}, {"SingleTop_tW_top_Incl", "Top"}, {"SingleTop_tW_antitop_Incl", "Top"}};
        
        //bkgNames ={"WJet", "QCD", "WW_pythia", "WZ_pythia", "ZZ_pythia", "DYJets10to50ToTauTau", "DYJetsToTauTau", "TTLL_powheg", "SingleTop_tW_top_Incl", "SingleTop_tW_antitop_Incl"};

        // Without Fake (for fake estimation)
        //bkgMap = {{"WJets_MG", "WJets"}, {"WW_pythia", "EWK"}, {"WZ_pythia", "EWK"}, {"ZZ_pythia", "EWK"}, {"DYJets10to50ToTauTau","EWK"}, {"DYJetsToTauTau","EWK"}, 
        //          {"TTLL_powheg", "Top"}, {"SingleTop_tW_top_Incl", "Top"}, {"SingleTop_tW_antitop_Incl", "Top"}};
        
        //bkgNames ={"WJets_MG", "WW_pythia", "WZ_pythia", "ZZ_pythia", "DYJets10to50ToTauTau", "DYJetsToTauTau", "TTLL_powheg", "SingleTop_tW_top_Incl", "SingleTop_tW_antitop_Incl"};

        //bkgMap = {{"WJets_MG", "WJets"}, {"WW_pythia", "EWK"}, {"WZ_pythia", "EWK"}, {"ZZ_pythia", "EWK"}, {"DYJets10to50ToTauTau","EWK"}, {"DYJetsToTauTau","EWK"}, 
        //          {"TTLL_powheg", "Top"} };
        
        //bkgNames ={"WJets_MG", "WW_pythia", "WZ_pythia", "ZZ_pythia", "DYJets10to50ToTauTau", "DYJetsToTauTau", "TTLL_powheg"};

        isEstimation = isEstimation_;
    
    }
    ~Hists(){}

    void setMassBindEdges();

    // Set Data histogram
    void setDataHist();

    // Set MC histograms
    void setMCHistInfo();
    void setMCHistInfo(std::pair<TString, TString> mcInfo);

    // Sum the same kind of backgrounds and return it
    TH1* getSummedHists(TString bkgTypes, TString steering, bool useAxis);

    // return Data-MC
    TH1* getFakeTemplate(bool doSmooth = false, double scale = 1.0);

    // Template fits
    void doDijetTemplateFit(TH1* hFakeTemplate, TString steering, bool useAxis);
    void doWjetTemplateFit();

    inline TH1* getDataHist()
    {
        return hData;
    } 
    inline TH1* getMCSig()
    {
        return hMCSig;
    }
    inline TH1* getFakeEstimation()
    {
        return hFakeEstimation;
    }

    void fillHistogram(TH1& hTarget, TH1& hSource, TString bin);
    void saveFakeEstimation(TString outHistName, TString bin = "");

    // Draw
    void setTHStack(THStack& hs, TH1& hMCtotal, TLegend& leg, TString steering, bool useAxis);
    TH1* getRawHist(TString histName, TString outHistName, TString steering, bool useAxis);
    TCanvas* drawHists(TString steering, bool useAxis, TString postfix = "");
};

#endif

