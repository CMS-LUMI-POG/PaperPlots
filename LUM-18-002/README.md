Some useful scripts for making the plots for the 2018 lumi PAS:

* compute_crossdetector.py: Performs the cross-detector comparison by computing the luminosity for each detector using the given normtag, looking at the average, and printing the difference for each from the normtag. Just give it a list of normtags and the appropriate json file to define the period over which the comparison should be run.

* makeHFAgingPlot.C: reads in the HF aging data from HFOCAging.csv (provided by David) and plots it. Run this as a compiled macro in ROOT (.x makeHFAgingPlot.C++).

* makeVdMLumiPlot.C: reads in the luminosity data for the luminometers during the VdM fill and makes the summary plot. See the macro for more information on how exactly this works.
