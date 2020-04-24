
void validation(TString var = "Mass")
{
    Hists detector = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/unfold_input_Muon_alpha_fake.root", "Detector", "muon", var, false);

    detector.setMCHistInfo();

    if(var == "Mass")
        detector.drawHists("mass[UO];pt[UOC0]", true);
    else
        detector.drawHists("pt[UO];mass[UO]", false);
}
