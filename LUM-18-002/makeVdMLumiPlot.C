// This script makes the nice version of the luminosity plot for the VdM fill in the PAS. Note -- make sure
// you compile this script (.x makeVdMLumiPlot.C++). Also note that this doesn't work properly under ROOT 5,
// so you'll have to use ROOT 6.  This expects that you have already created the csv files from brilcalc using
// the commands:
// brilcalc lumi -f 6868 --byls -u hz/ub -b "STABLE BEAMS" -o 6868_HFET.csv --normtag hfet18PAS
// similarly for HFOC (hfoc18PAS), PCC (pcc18PAS), BCM1F (bcm1f18PAS), PLT (pltReproc18PAS)

// This is very similar to the 2017 script. It doesn't need the special handling for PCC that the 2017 script
// has, which makes life a little simpler, but it does need special handling to cut out the region where CMS
// was out due to the fire alarm, so that part is somewhat more complicated.

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
#include "TGaxis.h"

static const int nLumis = 5;

// Note: at least the first of these should be present for all LSes in the data, otherwise
// the script will be very confused. Also PCC should be last because it needs special handling.
const char *lumiNames[nLumis] = {"HFET", "HFOC", "PLT", "BCM1F", "PCC"};
const int lColor[nLumis] = {kMagenta, kBlack, kRed, kBlue, 8};

int makeVdMLumiPlot(void) {
  bool firstLumi = true;
  std::map<std::pair<std::string,std::string>,int> lsMap;
  // lumisection number and lumi value for all the luminometers
  std::vector<float> lumiDataX[nLumis];
  std::vector<float> lumiDataY[nLumis];
  // subset of the above for the inset
  std::vector<float> lumiDataInsetX[nLumis];
  std::vector<float> lumiDataInsetY[nLumis];

  // define period of data to remove from plot
  int cutPeriodBegin = 750;
  int cutPeriodEnd = 1650;
  int totls;

  for (int i=0; i<nLumis; ++i) {
    std::string fileName = "6868_";
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
	// "fix" luminometers for bad period
	if (lsnum >= 680 && lsnum <= 1700)
	  lumiDel = 0;

	lsMap[std::make_pair(fields[0], fields[1])] = lsnum;

	// Don't put data from the bad period in the vector.
	if (lsnum <= cutPeriodBegin) {
	  lumiDataX[i].push_back(lsnum);
	  lumiDataY[i].push_back(lumiDel);
	} else if (lsnum > cutPeriodEnd) {
	  lumiDataX[i].push_back(lsnum-(cutPeriodEnd-cutPeriodBegin));
	  lumiDataY[i].push_back(lumiDel);
	}
	  
	if (lsnum >= 3313 && lsnum <= 3376) {
	  lumiDataInsetX[i].push_back(lsnum);
	  lumiDataInsetY[i].push_back(lumiDel);
	}
      } else {
	if (lsMap.count(std::make_pair(fields[0], fields[1])) > 0) {
	  int thisls = lsMap[std::make_pair(fields[0], fields[1])];

	  // "fix" luminometers for bad period
	  if (thisls >= 680 && thisls <= 1700)
	    lumiDel = 0;

	  // Don't put data from the bad period in the vector.
	  if (thisls <= cutPeriodBegin) {
	    lumiDataX[i].push_back(thisls);
	    lumiDataY[i].push_back(lumiDel);
	  } else if (thisls > cutPeriodEnd) {
	    lumiDataX[i].push_back(thisls-(cutPeriodEnd-cutPeriodBegin));
	    lumiDataY[i].push_back(lumiDel);
	  }

	  // also check to see if we want to put this in the inset
	  if (thisls >= 3313 && thisls <= 3376) {
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
    if (i==0) totls = lsnum;
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
  g[0]->GetYaxis()->SetTitleOffset(0.5);
  g[0]->SetLineColor(lColor[0]);
  g[0]->GetXaxis()->SetRangeUser(0, 2500);

  // Fix axis labels.
  for (int i=0; i<(int)lumiDataX[0].size(); i+=500) {
    int bin = g[0]->GetXaxis()->FindBin(i);
    int truenum;
    if (i<=cutPeriodBegin) {
      truenum = i;
    } else {
      truenum = i+(cutPeriodEnd-cutPeriodBegin);
    }

    std::ostringstream label;
    label << truenum;
    g[0]->GetXaxis()->SetBinLabel(bin, label.str().c_str());
  }
  g[0]->GetXaxis()->LabelsOption("h");
  g[0]->GetXaxis()->SetLabelSize(0.06);
  g[0]->GetXaxis()->SetNdivisions(300);

  // Yay a workaround for stupid ROOT things! So if you use ticks on this axis everything gets messed up.
  // Instead we draw ANOTHER axis just to have the ticks. Let's do that after drawing the boxes though.
  g[0]->GetXaxis()->SetTickLength(0);

  c1->Update();
  // Draw the shaded boxes corresponding to the area used for the cross-detector comparison.
  float begin1 = lsMap[std::make_pair("318982:6868", "7:7")];
  float end1 = lsMap[std::make_pair("318983:6868", "44:44")];
  float begin2 = lsMap[std::make_pair("319018:6868", "1:1")]-(cutPeriodEnd-cutPeriodBegin);
  float end2 = lsMap[std::make_pair("319018:6868", "48:48")]-(cutPeriodEnd-cutPeriodBegin);
  float begin3 = lsMap[std::make_pair("319019:6868", "1024:1024")]-(cutPeriodEnd-cutPeriodBegin);
  float end3 = lsMap[std::make_pair("319019:6868", "1087:1087")]-(cutPeriodEnd-cutPeriodBegin);
  TBox *b1 = new TBox(begin1, 0, end1, 9.0);
  b1->SetLineColor(18);
  b1->SetFillColor(18);
  b1->Draw();
  TBox *b2 = new TBox(begin2, 0, end2, 9.0);
  b2->SetLineColor(18);
  b2->SetFillColor(18);
  b2->Draw();
  TBox *b3 = new TBox(begin3, 0, end3, 9.0);
  b3->SetLineColor(18);
  b3->SetFillColor(18);
  b3->Draw();
  g[0]->Draw("same"); // redraw to get line on top of box again

  for (int i=1; i<nLumis; ++i) {
    g[i]->SetLineColor(lColor[i]);
    g[i]->Draw("same"); 
  }

  // draw ticks, as discussed above
  TGaxis *ticks = new TGaxis(0, c1->GetUymin(), 2500, c1->GetUymin(), 0, 2500, 510);
  ticks->SetLabelSize(0);
  ticks->Draw();

  // do the nifty little "cutout" bit
  TLine *cut1 = new TLine(cutPeriodBegin-10, 0, cutPeriodBegin+10, 0);
  cut1->SetLineColor(kWhite);
  cut1->Draw();
  TLine *cut2 = new TLine(cutPeriodBegin-20, -0.4, cutPeriodBegin, 0.4);
  cut2->Draw();
  TLine *cut3 = new TLine(cutPeriodBegin, -0.4, cutPeriodBegin+20, 0.4);
  cut3->Draw();

  TLegend *l = new TLegend(0.8, 0.6, 0.95, 0.9);
  for (int i=0; i<nLumis; ++i) {
    l->AddEntry(g[i], lumiNames[i], "L");
  }
  l->SetBorderSize(0);
  l->Draw();

  // CMS Preliminary
  TText *t1 = new TText(40, 10.0, "CMS Preliminary");
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
  gi[nLumis-1]->SetMinimum(8.3);
  gi[nLumis-1]->SetMaximum(8.9);
  gi[nLumis-1]->GetXaxis()->SetNdivisions(505);
  for (int i=0; i<nLumis-1; ++i) {
    gi[i]->SetLineColor(lColor[i]);
    gi[i]->Draw("same"); 
  }
  c1->Print("VdMFillLumi6868.pdf");
  c1->Print("VdMFillLumi6868.png");
  return 0;
}
