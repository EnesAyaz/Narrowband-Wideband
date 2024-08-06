opts.DataLines = [2, Inf];
opts.Delimiter = ",";
opts.VariableNames = ["Freq", "VOUT1", "VOUT2"];
opts.VariableTypes = ["string", "string", "double"];
opts.ExtraColumnsRule = "ignore";
opts.EmptyLineRule = "read";
opts = setvaropts(opts, ["Freq", "VOUT1"], "WhitespaceRule", "preserve");
opts = setvaropts(opts, ["Freq", "VOUT1"], "EmptyFieldRule", "auto");
tbl = readtable("C:\Github\Narrowband-Wideband\Spice Models\Design_wide_band.txt", opts);
Freq = tbl.Freq;
VOUT1 = tbl.VOUT1;
VOUT2 = tbl.VOUT2;
clear opts tbl