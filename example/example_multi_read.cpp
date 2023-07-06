#include <midas.hpp>

#undef NDEBUG
#include <assert.h>

#include <TCanvas.h>
#include <TFile.h>
#include <TH2.h>

#include <cstdlib>

const int bar_limit = 50;
const char hist_pattern[] = "hist_%06d";
const char can_pattern[] = "can_%06d";

void bar(int i)
{
    printf(".");
    if (i and (i + 1) % bar_limit == 0) printf("  %8d\n", i + 1);
}

void write_func()
{
    pandora::pandora* box = new pandora::pandora("box1");

    // fill with histograms
    char hname[100];
    for (int i = 0; i < 100; ++i)
    {
        sprintf(hname, hist_pattern, i);
        TH2F* h = box->reg_hist<TH2F>(hname, "Histogram - loop", 100, -5, 5, 100, -5, 5);

        for (int j = 0; j < 100 * 100; ++j)
            h->SetBinContent(j + 1, sqrt(j));

        bar(i);
    }

    // export box to file
    box->export_structure("example_multi.root", true);
}

void loop_read_func()
{
    pandora::pandora* box = new pandora::pandora("box1");

    // import from file and register in the boxtory
    // data will be stored in memory, file remains open
    TFile* f = box->import_structure("example_multi.root");

    // you can fetch specific object by its name
    char hname[100];
    for (int i = 0; i < 100; ++i)
    {
        sprintf(hname, hist_pattern, i);
        TH2F* h1 = dynamic_cast<TH2F*>(box->get_object(hname));
        // if failed, then objects are not read from file
        assert(h1 != nullptr);
    }

    delete box;

    // file must be closed by user
    f->Close();
}

int main(int argc, char** argv)
{
    int loops = 100;

    if (argc > 1) loops = atoi(argv[1]);

    printf("Create root file\n");
    write_func();

    printf("Read root file in %d loops\n", loops);
    for (int i = 0; i < loops; ++i)
    {
        loop_read_func();

        bar(i);
    }
}
