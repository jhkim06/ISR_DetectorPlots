
void validation(TString var = "Mass")
{
    Hists detector = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/unfold_input_Muon_fake.root", "Detector", "Muon", var, false);
    //Hists detector = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/unfold_input_Muon_fake.root", "Detector", "Muon", var, false);
    //Hists detector = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/muon/2017/unfold_input_Muon_ZptCorr_fake.root", "Detector", "Muon", var, false);

    detector.setMCHistInfo();

    if(var == "Mass")
        detector.drawHists("mass[UO];pt[UOC0]", true);
    else
        detector.drawHists("pt[UO];mass[UO]", false);
}
