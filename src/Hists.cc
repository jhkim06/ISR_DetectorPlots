#include "../include/Hists.h"

void Hists::setDataHist()
{
    cout << "Hists::setDataHist " << endl;
    hData = (TH1*) inFile->Get(histDir+"/"+var+"/"+"histo_Double"+channel_postfixData+"nominal");
}

void Hists::setMCHistInfo()
{

    int bkgSize = bkgNames.size();
    for(int i = 0; i < bkgSize; i++)
    {
        bkgTypes.push_back(bkgMap[bkgNames[i]]);
    }
}

void Hists::setMCHistInfo(std::pair<TString, TString> mcInfo)
{
    cout <<"Set " << mcInfo.first << endl;
    bkgNames.push_back(mcInfo.first);
    bkgTypes.push_back(mcInfo.second);
}

TCanvas* Hists::drawHists(TString steering, bool useAxis)
{
    TH1::AddDirectory(kFALSE);

    TH1* hMCtotal = NULL;
    TH1* hRatio = NULL;

    hData = getRawHist("histo_DoubleMuonnominal", "Data", steering, useAxis);
    hMCSig = getRawHist("histo_DYJetsToMuMunominal", "Signal", steering, useAxis);
    hMCtotal = (TH1*) hMCSig->Clone("hMCtotal");
    hRatio = (TH1*) hData->Clone("hRatio");

    TCanvas* c_out = new TCanvas("detector_level", "detector_level", 50, 50, 800, 700);
    c_out->Draw();
    c_out->cd();

    TPad *pad1 = new TPad("pad1","pad1",0,0.3,1,1);
    pad1->SetBottomMargin(0.01);
    pad1->SetTopMargin(0.1);
    pad1->SetTicks(1);
    pad1->SetLogy();
    if(var=="Mass")
        pad1->SetLogx();
    pad1->Draw();
    pad1->cd();

    hData->SetTitle("");
    hData->SetStats(false);
    hData->Draw("p9histe");
    hData->SetMarkerStyle(20);
    hData->SetMarkerSize(.7);
    hData->SetLineColor(kBlack);
    hData->GetYaxis()->SetTitle("Events/bin");
    hData->SetMaximum(1e8);
    hData->SetMinimum(1e-1);

    hMCSig->SetFillColor(kYellow);

    THStack* hsMC = new THStack("hsMC", "hsMC");
    setTHStack(*hsMC, *hMCtotal, steering, useAxis);
    hsMC->Add(hMCSig);

    hsMC->Draw("hist same");
    hData->Draw("p9histe same");

/*
    TLine massEdgeLine;
    if(var=="Mass")
    {
        for(int i = 0; i < massBinEdges.size(); i++)
        {
            if(i==0) continue;
            if(i==massBinEdges.size()-1) continue;
            massEdgeLine.DrawLine(massBinEdges[i], hData->GetMinimum(), massBinEdges[i], hData->GetMaximum());
        }
    }
*/
    c_out->cd();

    TPad *pad2 = new TPad("pad2","pad2",0,0,1,0.3);
    pad2->SetTopMargin(0.05);
    pad2->SetBottomMargin(0.2);
    if(var=="Mass")
        pad2->SetLogx();
    pad2->SetTicks(1);
    pad2->SetGridy(1);
    pad2->Draw();
    pad2->cd();

    hRatio->SetStats(false);
    hRatio->Divide(hMCtotal);
    hRatio->Draw("p9histe");
    hRatio->SetMarkerStyle(20);
    hRatio->SetMarkerSize(0.7);
    hRatio->SetLineColor(kBlack);
    hRatio->GetYaxis()->SetTitle("Data/MC");

    hRatio->SetMinimum(0.7);
    hRatio->SetMaximum(1.3);

    c_out->cd();
    c_out->SaveAs("detector_"+var+".pdf");

    return c_out;
}

TH1* Hists::getFakeTemplate(bool doSmooth)
{
    TH1::AddDirectory(kFALSE);
    TH1* hFakeTemplate = (TH1*) hData->Clone("FakeTemplate");
    hFakeTemplate->Add(hMCSig, -1.);
    hFakeTemplate->Add(hMCBkgs, -1.);

    //
    if(doSmooth)
        hFakeTemplate->Smooth();

    return hFakeTemplate; 
}

void Hists::doDijetTemplateFit(TH1* hFakeTemplate, TString steering, bool useAxis)
{

    double minFitRange = 0.;
    double maxFitRange = 0.;

    // FIXME Update for pT
    if(var=="Mass")
    {
        minFitRange = binDef->GetGlobalBinNumber(15.01, 10.);
        maxFitRange = binDef->GetGlobalBinNumber(199.99, 10.);
        cout << "min: " << minFitRange << " max: " << maxFitRange << endl;
    }

    RooRealVar fitVar(var, var, minFitRange, maxFitRange);

    RooDataHist *rhData = new RooDataHist("RooHist_data", "RooHistogram_data", fitVar, hData);
    RooDataHist *rhFakeTemplate = new RooDataHist("RooHist_QCDTemplate", "RooHistogram_QCDTemplate", fitVar, hFakeTemplate);

    TH1* hTop = getSummedHists("Top", steering, useAxis);
    TH1* hEWK = getSummedHists("EWK", steering, useAxis);

    RooDataHist *rhTop = new RooDataHist("RooHist_Top", "RooHistogram_Top", fitVar, hTop);
    RooDataHist *rhEWK = new RooDataHist("RooHist_EWK", "RooHistogram_EWK", fitVar, hEWK);
    RooDataHist *rhMCSig = new RooDataHist("RooHist_MCSig", "RooHistogram_MCSig", fitVar, hMCSig);

    //Convert RooDataHist to RooHistPdf
    RooHistPdf *pdfFakeTemplate = new RooHistPdf("pdfFakeTemplate", "Template from data-driven dijet", fitVar, *rhFakeTemplate, 0);
    RooHistPdf *pdfTop = new RooHistPdf("pdfTop", "Template from Top MC", fitVar, *rhTop, 0);
    RooHistPdf *pdfEWK = new RooHistPdf("pdfEWK", "Template from EWK MC", fitVar, *rhEWK, 0);
    RooHistPdf *pdfMCSig = new RooHistPdf("pdfMCSig", "Template from MCSig MC", fitVar, *rhMCSig, 0);

    // Construct model = n_ttbar * ttbar + n_WJets * WJets
    double N_Top = hTop->Integral(minFitRange, maxFitRange);
    double N_MCSig = hMCSig->Integral(minFitRange, maxFitRange);
    double N_Fake = hFakeTemplate->Integral(minFitRange, maxFitRange);
    double N_EWK = hEWK->Integral(minFitRange, maxFitRange);
    double ratio = hData->Integral(minFitRange, maxFitRange)/(N_Top + N_MCSig + N_Fake + N_EWK);
    N_Top *= ratio;
    N_MCSig *= ratio;
    N_Fake *= ratio;
    N_EWK *= ratio;

    RooRealVar n_Top("n_Top", "n_Top", N_Top, N_Top*0.9, N_Top*1.1);
    RooRealVar n_MCSig("n_MCSig", "n_MCSig", N_MCSig, N_MCSig*0.7, N_MCSig*1.1);
    RooRealVar n_Fake("n_Fake", "n_Fake", N_Fake, N_Fake*0.7, N_Fake*1.3);
    RooRealVar n_EWK("n_EWK", "n_EWK", N_EWK, N_EWK*0.9, N_Fake*1.1);
    RooAddPdf model("model","model", RooArgList(*pdfFakeTemplate, *pdfMCSig, *pdfTop, *pdfEWK), RooArgList(n_Fake, n_MCSig, n_Top, n_EWK));

    // Fit to data
    RooFitResult* r = model.fitTo( *rhData, Save());

    // FIXME
    TCanvas *c_fit = new TCanvas("c_fit", "", 800, 800);
    c_fit->cd();

    cout << "fake bf fit: " << N_Fake << endl;
    cout << "fake af fit: " << n_Fake.getVal() << endl;

    hFakeEstimation = (TH1*) hFakeTemplate->Clone("FakeEstimation");
    hFakeEstimation->Scale(n_Fake.getVal()/ N_Fake);

    //Top Pad
    TPad *c1_1 = new TPad("padc1_1","padc1_1",0.01,0.01,0.99,0.99);
    c1_1->Draw();
    c1_1->cd();
    c1_1->SetTopMargin(0.01);
    c1_1->SetBottomMargin(0.25);
    c1_1->SetRightMargin(0.03);
    c1_1->SetLeftMargin(0.12);
    c1_1->SetLogy();

    RooPlot* frame1 = fitVar.frame( Title(" ") ) ;
    rhData->plotOn(frame1, DataError(RooAbsData::SumW2));
    model.plotOn(frame1, Components("pdfFakeTemplate,pdfEWK,pdfTop,pdfMCSig"), LineColor(0), FillColor(kYellow), DrawOption("F") ); // Space NOT allowed in Components() !!
    model.plotOn(frame1, Components("pdfFakeTemplate,pdfEWK,pdfTop"), LineColor(0), FillColor(kBlue), DrawOption("F") );
    model.plotOn(frame1, Components("pdfFakeTemplate,pdfEWK"), LineColor(0), FillColor(kYellow+2), DrawOption("F") );
    model.plotOn(frame1, Components("pdfFakeTemplate"), LineColor(0), FillColor(kViolet+2), DrawOption("F") );
    rhData->plotOn(frame1, DataError(RooAbsData::SumW2));
    frame1->Draw(); // Draw frame
    frame1->GetYaxis()->SetTitle("Events");
    frame1->GetXaxis()->SetLabelSize(0);
    frame1->SetMinimum(1e-5);
    frame1->SetMaximum(1e8);

    //Bottom Pad
    TPad *c1_2 = new TPad("padc1_2","padc1_2",0.,0.01,1.,0.25);
    c1_2->Draw();
    c1_2->cd();
    c1_2->SetTopMargin(0.1);
    c1_2->SetBottomMargin(0.30);
    c1_2->SetRightMargin(0.03);
    c1_2->SetLeftMargin(0.12);
    c1_2->SetFillStyle(0);
    c1_2->SetGrid();

    TH1D *h_MC = (TH1D*)model.createHistogram("h_MC", fitVar);

    TH1D *h_data_ = (TH1D*) h_MC->Clone("h_data_");
    for(int ibin = 1; ibin < h_MC->GetNbinsX()+1; ibin++)
    {   
        h_data_->SetBinContent(ibin, hData->GetBinContent(ibin));
        h_data_->SetBinError(ibin, hData->GetBinError(ibin));
    }
    
    //Make ratio plot
    TH1D *h_ratio = (TH1D*)h_data_->Clone("ratio");
    h_ratio->Divide(h_data_, h_MC);
    h_ratio->SetTitle("");
    h_ratio->GetYaxis()->SetTitle("data/MC");
    h_ratio->SetMaximum( 2. );
    h_ratio->SetMinimum( 0. );
    h_ratio->SetMarkerSize(1.0);
    h_ratio->SetLineColor(kBlack);
    h_ratio->SetMarkerStyle(20);
    h_ratio->SetStats(kFALSE);

    h_ratio->Draw("p");
}

void Hists::saveFakeEstimation()
{
    TFile* fout = new TFile("Fake.root","recreate");
    fout->cd();
    TDirectory* dir = fout->mkdir(histDir+"/"+var);
    TDirectory* pdir = dir->GetDirectory(var); 
    pdir->cd();
    hFakeEstimation->SetName("histo_QCDnominal");
    hFakeEstimation->Write();

    fout->Close();
}

TH1* Hists::getSummedHists(TString bkgType, TString steering, bool useAxis)
{

    TH1::AddDirectory(kFALSE);
    cout << "Hists::getSummedHists" << endl;
    int bkgSize = bkgNames.size();
    bool firstBkg = true;
    TH1* hBkg = NULL;
    for(int i = 0; i < bkgSize; i++)
    {
        cout << i << " " << bkgTypes[i] << endl;
        if(firstBkg && bkgTypes[i] == bkgType)
        {
            cout << "first " << bkgTypes[i] << endl;
            hBkg = getRawHist("histo_"+bkgNames[i]+"nominal", bkgNames[i], steering, useAxis);
            firstBkg = false;
            continue;
        }
        if(!firstBkg && bkgTypes[i] == bkgType)
        {
            cout << "... " << bkgTypes[i] << endl;
            hBkg->Add(getRawHist("histo_"+bkgNames[i]+"nominal", bkgNames[i], steering, useAxis));
        }
    }
    return hBkg;
}

void Hists::setTHStack(THStack& hs, TH1& hMCtotal, TString steering, bool useAxis)
{
    TH1::AddDirectory(kFALSE);
    int bkgSize = bkgNames.size();

    // Count total number of each background type N
    map<TString, int> bkgTypeN;
    cout << "N bkg: " << bkgSize << endl;
    for(int i = 0; i < bkgSize; i++)
    {
        map<TString, int>::iterator it = bkgTypeN.find(bkgTypes[i]);
        if(it != bkgTypeN.end())
        {
            bkgTypeN[bkgTypes[i]]++;
        }
        else
        {
            cout << bkgTypes[i] << " first found" << endl;
            bkgTypeN[bkgTypes[i]] = 1;
        }
    }

    TH1* htemp = NULL;
    bool isFirstBkg = true;
    int nthBkg = 0;

    for(int i = 0; i < bkgSize; i++)
    {
        if(isEstimation && bkgTypes[i] == "Fake")
            continue;
        
        if(isFirstBkg)
        {
            htemp = getRawHist("histo_"+bkgNames[i]+"nominal", "h"+bkgNames[i], steering, useAxis);
            isFirstBkg = false;
            nthBkg++;

            hMCBkgs = (TH1*) htemp->Clone("hMCBkgs");
        }
        else
        {
            htemp->Add(getRawHist("histo_"+bkgNames[i]+"nominal", "h"+bkgNames[i], steering, useAxis));
            nthBkg++;

            hMCBkgs->Add(htemp);
        }

        // This type of backgrounds all added, so add them to THStack
        if(nthBkg == bkgTypeN[bkgTypes[i]])
        {
            cout << bkgTypes[i] << " " << bkgTypeN[bkgTypes[i]] << endl;
            htemp->SetFillColor(bkgColors[bkgTypes[i]]);
            hs.Add(htemp);
            hMCtotal.Add(htemp);

            isFirstBkg = true;
            nthBkg = 0;
        }
    }
}

TH1* Hists::getRawHist(TString histName, TString outHistName, TString steering, bool useAxis)
{
    TH1::AddDirectory(kFALSE);

    TH1* raw_hist = (TH1*)inFile->Get(histDir+"/"+var+"/"+histName);
    if(histName.Contains("DYJetsTo") && !histName.Contains("Tau"))
    {
        histName.ReplaceAll("DYJetsTo", "DYJets10to50To");
        raw_hist->Add((TH1*)inFile->Get(histDir+"/"+var+"/"+histName));
    }

    TH1* hist;
    hist = (TH1*) binDef->ExtractHistogram(outHistName, raw_hist, 0, useAxis, steering);

    delete raw_hist;
    return hist;
}
