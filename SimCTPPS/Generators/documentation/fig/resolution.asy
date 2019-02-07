import root;
import pad_layout;

string topDir = "../../test/m_X_1200/";
string f = topDir + "ppxzGeneratorValidation.root";

string methods[], m_labels[];
methods.push("single"); m_labels.push("\vbox{\hbox{single RP}\hbox{reconstruction}}");
methods.push("multi"); m_labels.push("\vbox{\hbox{multi RP}\hbox{reconstruction}}");

xSizeDef = 7cm;

TGraph_errorBar = None;

//----------------------------------------------------------------------------------------------------

void PrintCaption()
{
	NewPad(false);

	NewPadLabel("proton, sector 45");

	NewPadLabel("proton, sector 56");

	NewPadLabel("inv.~mass of X+Z");

	NewPadLabel("inv.~mass of X");
}

//----------------------------------------------------------------------------------------------------

PrintCaption();

for (int mi : methods.keys)
{
	NewRow();

	NewPadLabel(m_labels[mi]);

	NewPad("$\xi_{\rm reco} - \xi_{\rm simu}$");
	draw(RootGetObject(f, "after simulation/h_de_xi_"+methods[mi]+"_45"), "vl,lR", red);
	AttachLegend();

	NewPad("$\xi_{\rm reco} - \xi_{\rm simu}$");
	draw(RootGetObject(f, "after simulation/h_de_xi_"+methods[mi]+"_56"), "vl,lR", red);
	AttachLegend();

	NewPad("$m_{\rm reco} - m_{\rm simu}\ung{GeV}$");
	draw(RootGetObject(f, "after simulation/h_de_m_XZ_"+methods[mi]), "vl,lR", red);
	AttachLegend();

	NewPad("$m_{\rm reco} - m_{\rm simu}\ung{GeV}$");
	draw(RootGetObject(f, "after simulation/h_de_m_X_"+methods[mi]), "vl,lR", red);
	AttachLegend();
}

GShipout("resolution_histograms", hSkip=3mm, vSkip=1mm);

//----------------------------------------------------------------------------------------------------

PrintCaption();

for (int mi : methods.keys)
{
	NewRow();

	NewPadLabel(m_labels[mi]);

	NewPad("$\xi_{\rm simu}$", "RMS of $\xi_{\rm reco} - \xi_{\rm simu}$");
	draw(RootGetObject(f, "after simulation/g_rms_de_xi_vs_xi_"+methods[mi]+"_45"), "d0,p,l", red);
	xlimits(0., 0.2, Crop);

	NewPad("$\xi_{\rm simu}$", "RMS of $\xi_{\rm reco} - \xi_{\rm simu}$");
	draw(RootGetObject(f, "after simulation/g_rms_de_xi_vs_xi_"+methods[mi]+"_56"), "d0,p,l", red);
	xlimits(0., 0.2, Crop);

	NewPad("$m_{\rm simu}\ung{GeV}$", "RMS of $m_{\rm reco} - m_{\rm simu}\ung{GeV}$");
	draw(RootGetObject(f, "after simulation/g_rms_de_m_vs_m_XZ_"+methods[mi]), "d0,p,l", red);
	currentpad.xTicks = LeftTicks(100., 50.);
	xlimits(1200., 1500., Crop);

	NewPad("$m_{\rm simu}\ung{GeV}$", "RMS of $m_{\rm reco} - m_{\rm simu}\ung{GeV}$");
	draw(RootGetObject(f, "after simulation/g_rms_de_m_vs_m_X_"+methods[mi]), "d0,p,l", red);
	//xlimits(1190., 1210., Crop);
	currentpad.xTicks = LeftTicks(5., 1.);
}

GShipout("resolution_profiles", hSkip=3mm, vSkip=1mm);
