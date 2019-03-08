#!/usr/bin/env python

import os

normtags = ['hfoc18PAS', 'hfet18PAS', 'pcc18PAS', 'bcm1f18PAS', 'pltReproc18PAS']
json_file = "vdm2018.json"
output_name = "temp.csv"

# Scale factors: these were needed with the old normtags to scale the result of the old normtags to the final
# cross sections. Now that we actually have the final normtags with the final cross sections in them, these
# aren't necessary any more.
scale_factors = {'hfoc18v6': 805.9/803.99, 'hfet18v6': 2503.6/2508.09, 'pcc18v4': 5.982/5.99746, 'pltzero18v4Reproc': 261.8/259.98,
                 'bcm1fpcvd18test14': 197.84/198.53}

# Output
lumi = {}

print "Getting lumi, please wait..."
for n in normtags:
    brilcalc_command = 'brilcalc lumi -u /nb -i '+json_file+' --normtag '+n+' -o '+output_name
    if n in scale_factors:
        brilcalc_command += ' -n '+str(scale_factors[n])
    os.system(brilcalc_command)
    with open(output_name) as infile:
        while 1:
            line = infile.readline()
            if not line:
                break

            if line[0:6] == "#nfill":
                line = infile.readline()
                fields = line.split(",")
                lumi[n] = float(fields[4])
                break

os.remove(output_name)

avg_total = 0
for n in normtags:
    avg_total += lumi[n]
avg_total /= len(normtags)

for n in sorted(normtags):
    print "%s %.3f %+.1f%%" % (n, lumi[n], 100*(lumi[n]-avg_total)/avg_total)
