#include <midas.hpp>

#include <hellofitty.hpp>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>

#include <cstdlib>
#undef NDEBUG
#include <assert.h>

int main()
{
    pandora::pandora box_sig("test_sig");

    // import from file and register in the box
    // data will be stored in memory, file remains open
    auto f = box_sig.import_structure("example.root", true);

    if (!f) return -1;

    // box_sig.list_registered_objects();

    auto* ctx_sig = dynamic_cast<midas::context*>(box_sig.get_object("pcm_costhcm_sig_ctx"));
    if (!ctx_sig)
    {
        f->Close();
        return -1;
    }

    ctx_sig->print();
    auto dist_sig = midas::distribution(*ctx_sig, &box_sig);
    dist_sig.prepare();
    // dist_sig.transform([](TH1* h) { h->Print(); });
    // dist_sig.transform([](TCanvas* c) { c->Print(); });

    hf::fitter hf;
    hf.set_verbose(true);
    hf.init_from_file("example_in.txt", "example_out.txt");
    hf::fit_entry stdfit(1080, 1200);
    stdfit.add_function("gaus(0)");
    stdfit.add_function("pol5(3)");
    stdfit.set_param(0, 100, 0, 10000);
    stdfit.set_param(1, 1115, 1100, 1120);
    stdfit.set_param(2, 5);
    hf.set_generic_entry(stdfit);
    dist_sig.fit_cells_hists(hf, 0);
    dist_sig.finalize();
    // box_sig.list_registered_objects();

    hf.export_to_file();

    auto f2 = TFile::Open("example_fitted.root", "RECREATE");

    pandora::pandora box("test");
    box.import_structure("example.root", true);
    box.export_structure(f2, true);

    dist_sig.save(f2);

    f2->Close();
    // f->Close();

    // file must be closed by user
    // f->Close();  // FIXME bug here
}
