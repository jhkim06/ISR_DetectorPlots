void Pt_Test(TString massBin = "0")
{
    Hists opposite_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/LL_OS.root", "Detector", "Muon", "Pt", true);
    Hists same_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/LL_SS.root", "detector_level", "Muon", "Pt", true);

    //Hists opposite_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/TL_OS.root", "Detector", "Muon", "Pt", true);
    //Hists same_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/TL_SS.root", "Detector", "Muon", "Pt", true);

    opposite_sign.setMCHistInfo();
    same_sign.setMCHistInfo();

    same_sign.drawHists("pt[UO];mass[UOC"+massBin+"]", false);
    opposite_sign.drawHists("pt[UO];mass[UOC"+massBin+"]", false);
   
    TH1* qcd_template = same_sign.getFakeTemplate();
    opposite_sign.doDijetTemplateFit(qcd_template, "pt[UO];mass[UOC"+massBin+"]", false); 

    // saveFakeEstimation("histo_QCDnominal"); 
    // Get Mass bin,
    // Create histogram 
    opposite_sign.saveFakeEstimation("histo_QCDnominal", massBin);
    //opposite_sign.saveFakeEstimation("histo_WJetnominal", massBin);
}
