// This script makes the nice version of the luminosity plot for the VdM fill
// in the PAS. Note -- make sure you compile this script (.x makeVdMLumiPlot.C++).
// This expects that you have already created the csv files from
// brilcalc using the commands:
// brilcalc lumi -f 6016 --byls -u hz/ub -b "STABLE BEAMS" -o 6016_HFET.csv --normtag hfet17v8
// similarly for HFOC (hfoc17v8), PCC (pcc17v6), BCM1F (bcm1f17v6), PLT (pltzero17v24)

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include "TH1.h"
#include "TGraph.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TColor.h"
#include "TText.h"

static const int nLumis = 5;

// Note: at least the first of these should be present for all LSes in the data, otherwise
// the script will be very confused. Also PCC should be last because it needs special handling.
const char *lumiNames[nLumis] = {"HFET", "HFOC", "PLT", "BCM1F", "PCC"};

const int lColor[nLumis] = {kBlack, kMagenta, kRed, kBlue, 8};

int makeVdMLumiPlot(void) {
  bool firstLumi = true;
  std::map<std::pair<std::string,std::string>,int> lsMap;
  // lumisection number and lumi value for all the luminometers
  std::vector<float> lumiDataX[nLumis];
  std::vector<float> lumiDataY[nLumis];
  // subset of the above for LS 1700-1900 for the inset
  std::vector<float> lumiDataInsetX[nLumis];
  std::vector<float> lumiDataInsetY[nLumis];
  // Because the PCC data has a missing chunk in the middle, we have
  // to plot it as two separate TGraphs to avoid an awkward line connecting
  // the two pieces. Woo.
  std::vector<float> extraPCCDataX;
  std::vector<float> extraPCCDataY;

  for (int i=0; i<nLumis; ++i) {
    std::string fileName = "6016_";
    fileName.append(lumiNames[i]);
    fileName.append(".csv");
    std::ifstream csvFile(fileName.c_str());
    if (!csvFile.is_open()) {
      std::cerr << "ERROR: cannot open csv file: " << fileName << std::endl;
      return(1);
    }

    // Go through the lines of the file.
    int lsnum = 0;
    std::string line;
    while (1) {
      std::getline(csvFile, line);
      if (csvFile.eof()) break;
      if (line.empty()) continue; // skip blank lines
      if (line.at(0) == '#') continue; // skip comment lines

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

      std::stringstream lumiDelString(fields[5]);
      float lumiDel;
      lumiDelString >> lumiDel;
      // std::cout << "r/f " << fields[0] << " l/l " << fields[1] << " del " << lumiDel << " rec " << fields[6] << std::endl;
      
      if (firstLumi == true) {
	lsMap[std::make_pair(fields[0], fields[1])] = lsnum;
	lumiDataX[i].push_back(lsnum);
	lumiDataY[i].push_back(lumiDel);
	if (lsnum >= 1700 && lsnum < 1900) {
	  lumiDataInsetX[i].push_back(lsnum);
	  lumiDataInsetY[i].push_back(lumiDel);
	}
      } else {
	if (lsMap.count(std::make_pair(fields[0], fields[1])) > 0) {
	  int thisls = lsMap[std::make_pair(fields[0], fields[1])];
	  // PCC data needs special handling. We skip LS<1000 entirely
	  // because that data is bad. 1000-1500 is treated normally.
	  // >1500 is put in a separate graph.
	  if (i == nLumis - 1) {
	    if (thisls > 1000 && thisls < 1500) {
	      lumiDataX[i].push_back(thisls);
	      lumiDataY[i].push_back(lumiDel);
	    } else if (thisls > 1500) {
	      extraPCCDataX.push_back(thisls);
	      extraPCCDataY.push_back(lumiDel);
	    }
	  } else {
	    lumiDataX[i].push_back(thisls);
	    lumiDataY[i].push_back(lumiDel);
	  }
	  // also check to see if we want to put this in the inset
	  if (thisls >= 1700 && thisls < 1900) {
	    lumiDataInsetX[i].push_back(thisls);
	    lumiDataInsetY[i].push_back(lumiDel);
	  }
	} else {
	  std::cout << "Warning: found run/fill/LS " << fields[0] << " " << fields[1] << " not in list of LS from first luminometer" << std::endl;
	}
      }
      // finished storing this in the lumidata array
      ++lsnum;	
    } // line loop

    std::cout << "Processed " << lsnum << " LS for " << lumiNames[i] << std::endl;
    firstLumi = false;
  } // luminometer loop

  // Phew. Now put the data into some graphs and draw them.
  gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("c1", "c1", 1400, 600);
  c1->cd();
  gPad->SetRightMargin(0.25);

  TGraph *g[nLumis];
  for (int i=0; i<nLumis; ++i) {
    g[i] = new TGraph(lumiDataX[i].size(), lumiDataX[i].data(), lumiDataY[i].data());
  }

  g[0]->Draw("AL");
  g[0]->SetTitle("");
  g[0]->GetXaxis()->SetTitle("Luminosity section number since beginning of fill");
  g[0]->GetYaxis()->SetTitle("Instantaneous luminosity (Hz/#mub)");
  g[0]->GetXaxis()->SetLabelSize(0.04);
  g[0]->GetXaxis()->SetTitleSize(0.05);
  g[0]->GetXaxis()->SetTitleOffset(0.9);
  g[0]->GetYaxis()->SetLabelSize(0.04);
  g[0]->GetYaxis()->SetTitleSize(0.05);
  g[0]->GetYaxis()->SetTitleOffset(0.7);

  c1->Update();
  // Draw the shaded boxes corresponding to the area used for rescaling.
  float begin1 = lsMap[std::make_pair("300027:6016", "1:1")];
  float end1 = lsMap[std::make_pair("300027:6016", "112:112")];
  float begin2 = lsMap[std::make_pair("300043:6016", "1:1")];
  float end2 = lsMap[std::make_pair("300043:6016", "334:334")];
  TBox *b1 = new TBox(begin1, 0, end1, 3.0);
  b1->SetLineColor(18);
  b1->SetFillColor(18);
  b1->Draw();
  TBox *b2 = new TBox(begin2, 0, end2, 3.0);
  b2->SetLineColor(18);
  b2->SetFillColor(18);
  b2->Draw();
  g[0]->Draw("same"); // redraw to get line on top of box again

  for (int i=1; i<nLumis; ++i) {
    g[i]->SetLineColor(lColor[i]);
    g[i]->Draw("same"); 
  }
  TGraph *gie = new TGraph(extraPCCDataX.size(), extraPCCDataX.data(), extraPCCDataY.data());
  gie->SetLineColor(lColor[nLumis-1]);
  gie->Draw("same");

  TLegend *l = new TLegend(0.8, 0.6, 0.95, 0.9);
  for (int i=0; i<nLumis; ++i) {
    l->AddEntry(g[i], lumiNames[i], "L");
  }
  l->SetBorderSize(0);
  l->Draw();

  // CMS Preliminary
  TText *t1 = new TText(40, 3.1, "CMS Preliminary");
  t1->SetTextSize(0.05);
  t1->SetTextFont(62);
  t1->Draw();

  // Inset
  TPad *p2 = new TPad("p2", "p2", 0.77, 0.06, 0.98, 0.57);
  p2->SetLeftMargin(0.2);
  p2->SetRightMargin(0.01);
  p2->SetTopMargin(0.04);
  p2->SetBottomMargin(0.2);
  p2->Draw();
  p2->cd();
  TGraph *gi[nLumis];
  for (int i=0; i<nLumis; ++i) {
    gi[i] = new TGraph(lumiDataInsetX[i].size(), lumiDataInsetX[i].data(), lumiDataInsetY[i].data());
  }
  // Draw PCC first so it doesn't cover up everything else.
  gi[nLumis-1]->Draw("AL"); 
  gi[nLumis-1]->SetTitle("");
  gi[nLumis-1]->SetLineColor(lColor[nLumis-1]);
  gi[nLumis-1]->GetXaxis()->SetTitle("LS number");
  gi[nLumis-1]->GetYaxis()->SetTitle("Inst. luminosity (Hz/#mub)");
  gi[nLumis-1]->GetXaxis()->SetLabelSize(0.06);
  gi[nLumis-1]->GetXaxis()->SetTitleSize(0.06);
  gi[nLumis-1]->GetXaxis()->SetTitleOffset(0.9);
  gi[nLumis-1]->GetYaxis()->SetLabelSize(0.06);
  gi[nLumis-1]->GetYaxis()->SetTitleSize(0.06);
  gi[nLumis-1]->GetYaxis()->SetTitleOffset(1.5);
  gi[nLumis-1]->SetMinimum(2.70);
  gi[nLumis-1]->SetMaximum(2.84);
  for (int i=0; i<nLumis-1; ++i) {
    gi[i]->SetLineColor(lColor[i]);
    gi[i]->Draw("same"); 
  }
  c1->Print("VdMFillLumi.pdf");
  return 0;
}
