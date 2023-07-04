#include <midas.hpp>

#include <TCanvas.h>
#include <TFile.h>
#include <TH1.h>

#include <cstdlib>
#undef NDEBUG
#include <assert.h>

int main()
{
    // create factory
    RT::Pandora box("pcm_costhcm");

    // import from file and register in the factory
    // data will be stored in memory, file remains open
    auto f = box.importStructure("output.root");

    if (!f) return -1;

    // list of registered objects
    // box.listRegisteredObjects();

    auto* ctx = dynamic_cast<midas::context*>(box.getObject("pcm_costhcm_midas_ctx"));
    if (ctx)
    {
        ctx->print();
        auto fac = midas::distribution(*ctx);
        fac.importStructure(f, true);
        fac.prepare();
        fac.transform([](TH1* h) { h->Print(); });
        fac.transform([](TCanvas* c) { c->Print(); });
    }

    // file must be closed by user
    f->Close();
}
