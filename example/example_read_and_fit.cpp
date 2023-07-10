#include <midas.hpp>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>

#include <cstdlib>
#undef NDEBUG
#include <assert.h>

int main()
{
    pandora::pandora box("pcm_costhcm");

    // import from file and register in the box
    // data will be stored in memory, file remains open
    auto f = box.import_structure("output.root");

    if (!f) return -1;

    // list of registered objects
    // box.listRegisteredObjects();

    auto* ctx = dynamic_cast<midas::context*>(box.get_object("pcm_costhcm_midas_ctx"));
    if (ctx)
    {
        ctx->print();
        auto dist = midas::distribution(*ctx, &box);
        box.import_structure(f, true);
        dist.prepare();
        dist.transform([](TH1* h) { h->Print(); });
        dist.transform([](TCanvas* c) { c->Print(); });
    }

    // file must be closed by user
    f->Close();
}
