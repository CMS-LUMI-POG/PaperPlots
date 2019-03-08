// This script makes the nice version of the HF aging plot for the VdM fill in the PAS. Run this script
// compiled (.x makeHFAgingPlot.C++). You could also probably make it work uncompiled if you figure out a way
// around the fact that cint doesn't support std::vector::data().

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include "TH1.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TColor.h"
#include "TText.h"
#include "TLine.h"

int makeHFAgingPlot(void) {
  const char *inputFile = "HFOCAging.csv";
  std::vector<float> xvals;
  std::vector<float> xerrs;
  std::vector<float> yvals;
  std::vector<float> yerrs;

  std::ifstream csvFile(inputFile);
  if (!csvFile.is_open()) {
    std::cerr << "ERROR: cannot open csv file: " << inputFile << std::endl;
    return(1);
  }

  // Go through the lines of the file.
  std::string line;
  while (1) {
    std::getline(csvFile, line);
    if (csvFile.eof()) break;
    if (line.empty()) continue; // skip blank lines
    if (!isdigit(line.at(0))) continue; // skip header (or other non-numeric) lines

    // Break into fields
    std::stringstream ss(line);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(ss, field, ','))
      fields.push_back(field);

    if (fields.size() != 9) {
      std::cout << "Malformed line in csv file: " << line << std::endl;
      continue;
    }

    std::stringstream xstr(fields[1]);
    std::stringstream ystr(fields[7]);
    std::stringstream yerrstr(fields[8]);
    float x, xerr, y, yerr;
    xstr >> x;
    xerr = 0;
    ystr >> y;
    yerrstr >> yerr;
      
    xvals.push_back(x);
    xerrs.push_back(xerr);
    yvals.push_back(y);
    yerrs.push_back(yerr);
	
  }
 
  // Phew. Now put the data into some graphs and draw them.
  gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("c1", "c1", 800, 600);
  c1->cd();

  TGraphErrors *g = new TGraphErrors(xvals.size(), xvals.data(), yvals.data(), xerrs.data(), yerrs.data());
  g->Draw("APE");
  g->SetTitle("");
  g->SetMarkerStyle(kFullCircle);
  g->GetXaxis()->SetTitle("Integrated luminosity (fb^{-1})");
  g->GetYaxis()->SetTitle("Relative efficiency (HFOC)");
  g->GetXaxis()->SetLabelSize(0.04);
  g->GetXaxis()->SetTitleSize(0.05);
  g->GetXaxis()->SetTitleOffset(0.9);
  g->GetYaxis()->SetLabelSize(0.04);
  g->GetYaxis()->SetTitleSize(0.05);
  g->GetYaxis()->SetTitleOffset(0.9);
  g->SetMarkerColor(kBlue);
  g->SetLineColor(kBlue);

  g->Fit("pol1", "", "", 97, 167);

  TF1* f = g->GetFunction("pol1");
  f->SetLineWidth(4);

  c1->Update();

  // CMS Preliminary
  TText *t1 = new TText(140, 1.1, "CMS Preliminary");
  t1->SetTextSize(0.05);
  t1->SetTextFont(62);
  t1->Draw();

  // Add line and text for the VdM scan
  TLine *l1 = new TLine(121.658, 0.9, 121.658, 1.1);
  l1->SetLineColor(kViolet);
  l1->SetLineWidth(5);
  l1->Draw();
  TText *t2 = new TText(123, 1.08, "2018 VdM scan");
  t2->SetTextColor(kViolet);
  t2->SetTextSize(0.03);
  t2->Draw();

  TLegend *leg = new TLegend(0.12, 0.12, 0.5, 0.22);
  leg->AddEntry(g, "Emittance scan results", "PE");
  leg->AddEntry(f, "Linear fit", "L");
  leg->SetBorderSize(0);
  leg->Draw();

  c1->Print("HFOCAging2018.png");
  c1->Print("HFOCAging2018.pdf");
  return 0;
}
