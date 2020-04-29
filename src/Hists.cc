#include "../include/Hists.h"
#include "./tdrStyle/tdrstyle.C"
#include "./tdrStyle/CMS_lumi.C"

void Hists::setMassBindEdges()
{
    const TVectorD* temp_tvecd = binDef->GetDistributionBinning(1);
    const Double_t* massBins = temp_tvecd->GetMatrixArray();
    int nMassBinEdges = temp_tvecd->GetNrows();

    if(massBinEdges.size() == 0)
    {
        for(int i = 0 ; i < nMassBinEdges; i++)
        {
            massBinEdges.push_back(massBins[i]);
            cout << i << " th mass bin edge: " << massBins[i] << endl;
        }
    }
    else
    {
        cout << "Hists::setMassBindEdges massBinEdges already set." << endl;
    }

}

void Hists::setDataHist()
{
    cout << "Hists::setDataHist " << endl;
    hData = (TH1*) inFile->Get(histDir+"/"+var+"/"+"histo_"+channel_postfixData+"nominal");
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

TCanvas* Hists::drawHists(TString steering, bool useAxis, TString postfix)
{

    setTDRStyle();
    writeExtraText = true;
    extraText  = "work in progress";

    TH1::AddDirectory(kFALSE);
    
    double minX = 50.;
    double maxX = 320.;

    if(!isEstimation)
    {
        if(channel=="Muon" && var =="Mass")
            minX = 40.;
    }

    TH1* hMCtotal = NULL;
    TH1* hRatio = NULL;

    cout << "Hists::drawHists get Data and MC histograms" << endl;
    hData = getRawHist("histo_"+channel_postfixData+"nominal", "Data", steering, useAxis);
    hMCSig = getRawHist("histo_DYJetsTo"+channel_postfixMC+"nominal", "Signal", steering, useAxis);
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
    //if(var=="Mass")
    //    pad1->SetLogx();
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

    if(!isEstimation && var=="Mass") hData->GetXaxis()->SetRangeUser(minX, maxX);
    hMCSig->SetFillColor(kYellow);

    TLegend* leg = new TLegend(0.65, 0.65, 0.85, 0.85,"","brNDC");
    //leg->SetNColumns(2);
    leg->SetTextSize(0.04);
    leg->SetFillStyle(0); // transparent
    leg->SetBorderSize(0);

    leg->AddEntry(hData, "Data", "pe");
    leg->AddEntry(hMCSig, "DY", "F");

    THStack* hsMC = new THStack("hsMC", "hsMC");
    setTHStack(*hsMC, *hMCtotal, *leg, steering, useAxis);
    hsMC->Add(hMCSig);

    leg->Draw();

    hsMC->Draw("hist same");
    hData->Draw("p9histe same");
    pad1->RedrawAxis();

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
    //if(var=="Mass")
    //    pad2->SetLogx();
    pad2->SetTicks(1);
    pad2->SetGridy(1);
    pad2->Draw();
    pad2->cd();

    hRatio->GetXaxis()->SetTitle("Mass [GeV]");
    if(var=="Pt")
        hRatio->GetXaxis()->SetTitle("p_{T} bin index");
    if(isEstimation)
        hRatio->GetXaxis()->SetTitle("Bin index");


    hRatio->SetStats(false);
    hRatio->Divide(hMCtotal);
    hRatio->Draw("p9histe");
    hRatio->SetMarkerStyle(20);
    hRatio->SetMarkerSize(0.7);
    hRatio->SetLineColor(kBlack);
    hRatio->GetYaxis()->SetTitle("Data/MC");

    if(!isEstimation && var=="Mass") hRatio->GetXaxis()->SetRangeUser(minX, maxX);
    hRatio->GetXaxis()->SetNdivisions(510);

    hRatio->SetMinimum(0.7);
    hRatio->SetMaximum(1.3);


    c_out->cd();
    CMS_lumi( c_out, 7, 0 );

    c_out->SaveAs("detector_"+var+"_"+channel+"_"+postfix+".pdf");

    return c_out;
}

TH1* Hists::getFakeTemplate(bool doSmooth, double scale)
{
    TH1::AddDirectory(kFALSE);
    TH1* hFakeTemplate = (TH1*) hData->Clone("FakeTemplate");
    hMCSig->Scale(scale); // FIXME How to handle big normalisation differences  
    hFakeTemplate->Add(hMCSig, -1.);
    hFakeTemplate->Add(hMCBkgs, -1.);

    //
    if(doSmooth)
        hFakeTemplate->Smooth();

    return hFakeTemplate;
}

void Hists::doDijetTemplateFit(TH1* hFakeTemplate, TString steering, bool useAxis)
{

    setTDRStyle();                                                                                                                                                                                                                                                                                                            
    writeExtraText = true;                                                                                                                                                                                                                                                                                                    
    extraText  = "work in progress";     

    double minFitRange = 0.;
    double maxFitRange = 0.;

    if(var=="Mass")
    {
        minFitRange = binDef->GetGlobalBinNumber(15.01, 10.);
        maxFitRange = binDef->GetGlobalBinNumber(199.99, 10.);
        cout << "min: " << minFitRange << " max: " << maxFitRange << endl;
    }
    else
    {
        minFitRange = 1.;
        maxFitRange = 9.;
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
    c1_1->SetTopMargin(0.1);
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
    frame1->SetMinimum(1e-1);
    frame1->SetMaximum(1e8);
    frame1->GetXaxis()->SetTitle("");

 
    hFakeTemplate->SetFillColor(kViolet+2);   
    hEWK->SetFillColor(kYellow+2);
    hTop->SetFillColor(kBlue);
    TLegend* legend = new TLegend(0.65, 0.65, 0.85, 0.85,"","brNDC");  
    legend->SetTextSize(0.04);
    legend->SetFillStyle(0); // transparent
    legend->SetBorderSize(0);
    legend->AddEntry(hData, "Data", "pe");
    legend->AddEntry(hEWK, "EWK", "F");
    legend->AddEntry(hTop, "Top", "F");
    legend->AddEntry(hFakeTemplate, "Fake", "F");
    legend->Draw();

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

    h_ratio->GetXaxis()->SetTitle("Bin index");

    h_ratio->Draw("p");

    c_fit->cd();
    CMS_lumi( c1_1, 7, 0 );     
    c_fit->SaveAs("fit_"+var+"_"+channel+".pdf"); 
}

void Hists::saveFakeEstimation(TString outHistName, TString bin)
{

    TFile* fout = new TFile("Fake.root","UPDATE");
    fout->cd();
    TDirectory* dir = fout->GetDirectory(histDir+"/"+var);
    if(dir == nullptr)
    {
        cout << "Directory not exist. " << endl;
        dir = fout->mkdir(histDir+"/"+var);
        dir = dir->GetDirectory(var);
        dir->cd();

    }
    else
    {
        cout << "Directory exits" << endl;
        dir->cd();
    }

    if(var=="Pt")
    {
        TH1* htemp = NULL;
        if(dir->Get(outHistName))
        {
            // Update existing histogram
            cout << outHistName << " exist." << endl;
            htemp = (TH1*) dir->Get(outHistName);
            fillHistogram(*htemp, *hFakeEstimation, bin);
        }
        else
        {
            // Create histogram
            htemp = binDef->CreateHistogram(outHistName);
            fillHistogram(*htemp, *hFakeEstimation, bin);
        }
        htemp->Write();

    }
    else
    {
        hFakeEstimation->SetName(outHistName);
        hFakeEstimation->Write();
    }

    fout->Close();
}

void Hists::fillHistogram(TH1& hTarget, TH1& hSource, TString bin)
{
    int NbinSource = hSource.GetNbinsX();

    int nthMassBin = bin.Atoi();
    int firstTargetBin = binDef->GetGlobalBinNumber(0., massBinEdges[nthMassBin]); 
    for(int i = 0; i < NbinSource; i++)
    {
        hTarget.SetBinContent(firstTargetBin+i, hSource.GetBinContent(i+1));
        hTarget.SetBinError(firstTargetBin+i, hSource.GetBinError(i+1));
    }
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

void Hists::setTHStack(THStack& hs, TH1& hMCtotal, TLegend& leg, TString steering, bool useAxis)
{
    TH1::AddDirectory(kFALSE);
    int bkgSize = bkgNames.size();

    // Count total number of each background type N
    map<TString, int> bkgTypeN;
    cout << "N bkg: " << bkgSize << endl;
    for(int i = 0; i < bkgSize; i++)
    {
        //if(!isEstimation && bkgNames[i] == "WJets_MG") continue;

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
    cout << "All bkg types set" << endl;

    TH1* htemp = NULL;
    bool isFirstBkg = true;
    int nthBkg = 0;

    for(int i = 0; i < bkgSize; i++)
    {
        //if(!isEstimation && bkgNames[i] == "WJets_MG") continue;
        cout << i << " " << "histo_"+bkgNames[i]+"nominal" << endl;
        if(isEstimation && bkgTypes[i] == "Fake")
            continue;

        if(isFirstBkg)
        {
            htemp = getRawHist("histo_"+bkgNames[i]+"nominal", "h"+bkgNames[i], steering, useAxis);
            if(htemp == NULL) continue;
            isFirstBkg = false;
            nthBkg++;

            hMCBkgs = (TH1*) htemp->Clone("hMCBkgs");
        }
        else
        {
            htemp->Add(getRawHist("histo_"+bkgNames[i]+"nominal", "h"+bkgNames[i], steering, useAxis));
            if(htemp == NULL) continue;
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

            leg.AddEntry(htemp, bkgTypes[i], "F");

            isFirstBkg = true;
            nthBkg = 0;
        }
    }
}

TH1* Hists::getRawHist(TString histName, TString outHistName, TString steering, bool useAxis)
{
    TH1::AddDirectory(kFALSE);

    TH1* raw_hist = (TH1*)inFile->Get(histDir+"/"+var+"/"+histName);
    if(raw_hist == NULL) return NULL;

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
