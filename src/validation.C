
void validation()
{
    Hists detector = Hists("/home/jhkim/ISR_Run2/fake/dijet/electron/2016/data/unfold_input_Muon_new_beta.root", "Detector", "muon", "Mass", false);

    detector.setMCHistInfo();

    detector.drawHists("mass[UO];pt[UOC0]", true);
}
