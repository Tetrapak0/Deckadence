#include "../../include/GUI.hpp"

#include <algorithm>

const vector<ImWchar> gui_get_glyph_ranges() {
    int lat = 0;
    int cns = 0;
    int cnf = 0;
    int cyr = 0;
    int grk = 0;
    int jap = 0;
    int kor = 0;
    int tha = 0;
    int vnm = 0;
    ImFontAtlas ifa;
    const ImWchar* gr_lat = ifa.GetGlyphRangesDefault(&lat);
    const ImWchar* gr_cns = ifa.GetGlyphRangesChineseSimplifiedCommon(&cns);
    const ImWchar* gr_cnf = ifa.GetGlyphRangesChineseFull(&cnf);
    const ImWchar* gr_cyr = ifa.GetGlyphRangesCyrillic(&cyr);
    const ImWchar* gr_grk = ifa.GetGlyphRangesGreek(&grk);
    const ImWchar* gr_jap = ifa.GetGlyphRangesJapanese(&jap);
    const ImWchar* gr_kor = ifa.GetGlyphRangesKorean(&kor);
    const ImWchar* gr_tha = ifa.GetGlyphRangesThai(&tha);
    const ImWchar* gr_vnm = ifa.GetGlyphRangesVietnamese(&vnm);
    const ImWchar* grs[9] = {gr_lat, gr_cns, gr_cnf, gr_cyr, gr_grk, gr_jap, gr_kor, gr_tha, gr_vnm};
    const int      grc[9] = {lat, cns, cnf, cyr, grk, jap, kor, tha, vnm};
    vector<ImWchar*> grfarr;
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < grc[i]; ++j) {
            ImWchar pair[2] = {grs[i][j], grs[i][++j]};
            grfarr.push_back(pair);
        }
    }

    vector<ImWchar> grf;
    for (int i = 0; i < grfarr.size(); ++i) {
        grf.push_back(grfarr[i][0]);
        grf.push_back(grfarr[i][1]);
    }
    grf.push_back(0);

    return grf;
}
