
void Test()
{
    // For Dijet estimation
    Hists opposite_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/LL_OS.root", "Detector", "Muon", "Mass", true);
    Hists same_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/LL_SS.root", "detector_level", "Muon", "Mass", true);

    // For WJet estimation
    //Hists opposite_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/TL_OS.root", "Detector", "Muon", "Mass", true);
    //Hists same_sign = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2018/TL_SS.root", "Detector", "Muon", "Mass", true);

    opposite_sign.setMCHistInfo();
    same_sign.setMCHistInfo();

    same_sign.drawHists("mass[UO];pt[UOC0]", false, "SS");
    opposite_sign.drawHists("mass[UO];pt[UOC0]", false, "OS");
   
    TH1* qcd_template = same_sign.getFakeTemplate();
    opposite_sign.doDijetTemplateFit(qcd_template, "mass[UO];pt[UOC0]", false); 

    opposite_sign.saveFakeEstimation("histo_QCDnominal");
    //opposite_sign.saveFakeEstimation("histo_WJetnominal");
}
