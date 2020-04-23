
void validation()
{
    Hists detector = Hists("/home/jhkim/ISR_Run2/fake/FakeRate/data/unfold_input_Muon_gamma.root", "Detector", "muon", "Mass", false);

    detector.setMCHistInfo();

    detector.drawHists("mass[UO];pt[UOC0]", true);
}
