/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2017 Los Alamos National Security, LLC
 * All rights reserved
 *~-------------------------------------------------------------------------~~*/

#include <cinchtest.h>

#include <flecsi/utils/hash.h>

// =============================================================================
// Test various aspects of flecsi::utils::hash
// =============================================================================

TEST(hash, all) {

  using flecsi::utils::string_hash;

  if (sizeof(std::size_t) == 8) {
    EXPECT_EQ(string_hash<std::size_t>("", 0), 0UL);
    EXPECT_EQ(string_hash<std::size_t>("a", 1), 579863930UL);
    EXPECT_EQ(string_hash<std::size_t>("bc", 2), 1804308674789032UL);
    EXPECT_EQ(string_hash<std::size_t>("def", 3), 17347935192790221998UL);
    EXPECT_EQ(string_hash<std::size_t>("ghij", 4), 2206241549843021049UL);
    EXPECT_EQ(string_hash<std::size_t>("klmno", 5), 12445193748225208824UL);

    EXPECT_EQ(string_hash<std::size_t>("", 0), 0UL);
    EXPECT_EQ(string_hash<std::size_t>("1", 1), 344982506UL);
    EXPECT_EQ(string_hash<std::size_t>("12", 2), 1085224634034564UL);
    EXPECT_EQ(string_hash<std::size_t>("123", 3), 1185801406734518843UL);
    EXPECT_EQ(string_hash<std::size_t>("1234", 4), 13378554461280252461UL);
    EXPECT_EQ(string_hash<std::size_t>("12345", 5), 5585044181051024734UL);
  }

  // A list of strings that are used in FleCSI projects and that have previously
  // led to hash collisions
  std::vector<std::string> strs;
  strs.push_back("Faii_a_evap");
  strs.push_back("Faii_a_lat");
  strs.push_back("Faii_a_lwup");
  strs.push_back("Faii_a_sen");
  strs.push_back("Faii_a_swnet");
  strs.push_back("Faii_a_taux");
  strs.push_back("Faii_a_tauy");
  strs.push_back("Faii_i_evap");
  strs.push_back("Faii_i_lat");
  strs.push_back("Faii_i_lwup");
  strs.push_back("Faii_i_sen");
  strs.push_back("Faii_i_swnet");
  strs.push_back("Faii_i_taux");
  strs.push_back("Faii_i_tauy");
  strs.push_back("Fall_a_evap");
  strs.push_back("Fall_a_lat");
  strs.push_back("Fall_a_lwup");
  strs.push_back("Fall_a_sen");
  strs.push_back("Fall_a_swnet");
  strs.push_back("Fall_a_taux");
  strs.push_back("Fall_a_tauy");
  strs.push_back("Fall_l_evap");
  strs.push_back("Fall_l_flxdst0");
  strs.push_back("Fall_l_flxdst1");
  strs.push_back("Fall_l_flxdst2");
  strs.push_back("Fall_l_flxdst3");
  strs.push_back("Fall_l_lat");
  strs.push_back("Fall_l_lwup");
  strs.push_back("Fall_l_sen");
  strs.push_back("Fall_l_swnet");
  strs.push_back("Fall_l_taux");
  strs.push_back("Fall_l_tauy");
  strs.push_back("Faox_a_evap");
  strs.push_back("Faox_a_lat");
  strs.push_back("Faox_a_lwup");
  strs.push_back("Faox_a_sen");
  strs.push_back("Faox_a_taux");
  strs.push_back("Faox_a_tauy");
  strs.push_back("Faox_o_evap");
  strs.push_back("Faox_o_lat");
  strs.push_back("Faox_o_lwup");
  strs.push_back("Faox_o_sen");
  strs.push_back("Faox_o_taux");
  strs.push_back("Faox_o_tauy");
  strs.push_back("Faxa_a_lwdn");
  strs.push_back("Faxa_a_rainc");
  strs.push_back("Faxa_a_rainl");
  strs.push_back("Faxa_a_snowc");
  strs.push_back("Faxa_a_snowl");
  strs.push_back("Faxa_a_swndf");
  strs.push_back("Faxa_a_swndr");
  strs.push_back("Faxa_a_swnet");
  strs.push_back("Faxa_a_swvdf");
  strs.push_back("Faxa_a_swvdr");
  strs.push_back("Faxa_i_lwdn");
  strs.push_back("Faxa_i_rainc");
  strs.push_back("Faxa_i_rainl");
  strs.push_back("Faxa_i_snowc");
  strs.push_back("Faxa_i_snowl");
  strs.push_back("Faxa_i_swndf");
  strs.push_back("Faxa_i_swndr");
  strs.push_back("Faxa_i_swnet");
  strs.push_back("Faxa_i_swvdf");
  strs.push_back("Faxa_i_swvdr");
  strs.push_back("Faxa_l_lwdn");
  strs.push_back("Faxa_l_rainc");
  strs.push_back("Faxa_l_rainl");
  strs.push_back("Faxa_l_snowc");
  strs.push_back("Faxa_l_snowl");
  strs.push_back("Faxa_l_swndf");
  strs.push_back("Faxa_l_swndr");
  strs.push_back("Faxa_l_swvdf");
  strs.push_back("Faxa_l_swvdr");
  strs.push_back("Faxa_o_lwdn");
  strs.push_back("Faxa_o_rainc");
  strs.push_back("Faxa_o_rainl");
  strs.push_back("Faxa_o_snowc");
  strs.push_back("Faxa_o_snowl");
  strs.push_back("Faxa_o_swndf");
  strs.push_back("Faxa_o_swndr");
  strs.push_back("Faxa_o_swnet");
  strs.push_back("Faxa_o_swvdf");
  strs.push_back("Faxa_o_swvdr");
  strs.push_back("Faxx_a_evap");
  strs.push_back("Faxx_a_lat");
  strs.push_back("Faxx_a_lwup");
  strs.push_back("Faxx_a_sen");
  strs.push_back("Faxx_a_taux");
  strs.push_back("Faxx_a_tauy");
  strs.push_back("Fioi_avg_melth");
  strs.push_back("Fioi_avg_meltw");
  strs.push_back("Fioi_avg_salt");
  strs.push_back("Fioi_i_melth");
  strs.push_back("Fioi_i_meltw");
  strs.push_back("Fioi_i_salt");
  strs.push_back("Fioi_i_swpen");
  strs.push_back("Fioi_i_tauxo");
  strs.push_back("Fioi_i_tauyo");
  strs.push_back("Fioi_o_melth");
  strs.push_back("Fioi_o_meltw");
  strs.push_back("Fioi_o_salt");
  strs.push_back("Fioi_o_swpen");
  strs.push_back("Fioi_o_tauxo");
  strs.push_back("Fioi_o_tauyo");
  strs.push_back("Fioo_i_frazil");
  strs.push_back("Fioo_i_meltp");
  strs.push_back("Fioo_o_frazil");
  strs.push_back("Fioo_o_meltp");
  strs.push_back("Flrl_l_rof0");
  strs.push_back("Flrl_l_rof1");
  strs.push_back("Flrl_l_rof2");
  strs.push_back("Flrl_l_rof3");
  strs.push_back("Flrl_l_rof4");
  strs.push_back("Flrr_l_rof0");
  strs.push_back("Flrr_l_rof1");
  strs.push_back("Flrr_l_rof2");
  strs.push_back("Forr_avg_rof0");
  strs.push_back("Forr_avg_rof1");
  strs.push_back("Forr_o_rof0");
  strs.push_back("Forr_o_rof1");
  strs.push_back("Foxx_avg_evap");
  strs.push_back("Foxx_avg_lat");
  strs.push_back("Foxx_avg_lwdn");
  strs.push_back("Foxx_avg_lwup");
  strs.push_back("Foxx_avg_rain");
  strs.push_back("Foxx_avg_sen");
  strs.push_back("Foxx_avg_snow");
  strs.push_back("Foxx_avg_swnet");
  strs.push_back("Foxx_avg_taux");
  strs.push_back("Foxx_avg_tauy");
  strs.push_back("Foxx_o_evap");
  strs.push_back("Foxx_o_lat");
  strs.push_back("Foxx_o_lwdn");
  strs.push_back("Foxx_o_lwup");
  strs.push_back("Foxx_o_rain");
  strs.push_back("Foxx_o_sen");
  strs.push_back("Foxx_o_snow");
  strs.push_back("Foxx_o_swnet");
  strs.push_back("Foxx_o_taux");
  strs.push_back("Foxx_o_tauy");
  strs.push_back("Sa_a_anidf");
  strs.push_back("Sa_a_anidr");
  strs.push_back("Sa_a_avsdf");
  strs.push_back("Sa_a_avsdr");
  strs.push_back("Sa_a_dens");
  strs.push_back("Sa_a_pbot");
  strs.push_back("Sa_a_pslv");
  strs.push_back("Sa_a_ptem");
  strs.push_back("Sa_a_shum");
  strs.push_back("Sa_a_Tbot");
  strs.push_back("Sa_a_topo");
  strs.push_back("Sa_a_U");
  strs.push_back("Sa_a_V");
  strs.push_back("Sa_avg_pbot");
  strs.push_back("Sa_a_z");
  strs.push_back("Sa_i_dens");
  strs.push_back("Sa_i_ptem");
  strs.push_back("Sa_i_shum");
  strs.push_back("Sa_i_Tbot");
  strs.push_back("Sa_i_U");
  strs.push_back("Sa_i_V");
  strs.push_back("Sa_i_z");
  strs.push_back("Sa_l_anidf");
  strs.push_back("Sa_l_anidr");
  strs.push_back("Sa_l_avsdf");
  strs.push_back("Sa_l_avsdr");
  strs.push_back("Sa_l_pbot");
  strs.push_back("Sa_l_ptem");
  strs.push_back("Sa_l_shum");
  strs.push_back("Sa_l_Tbot");
  strs.push_back("Sa_l_U");
  strs.push_back("Sa_l_V");
  strs.push_back("Sa_l_z");
  strs.push_back("Sa_o_dens");
  strs.push_back("Sa_o_pbot");
  strs.push_back("Sa_o_ptem");
  strs.push_back("Sa_o_shum");
  strs.push_back("Sa_o_Tbot");
  strs.push_back("Sa_o_U");
  strs.push_back("Sa_o_V");
  strs.push_back("Sa_o_z");
  strs.push_back("Si_a_anidf");
  strs.push_back("Si_a_anidr");
  strs.push_back("Si_a_avsdf");
  strs.push_back("Si_a_avsdr");
  strs.push_back("Si_a_ifrac");
  strs.push_back("Si_a_qref");
  strs.push_back("Si_a_snowh");
  strs.push_back("Si_a_tref");
  strs.push_back("Si_a_Tsfc");
  strs.push_back("Si_a_U10");
  strs.push_back("Si_avg_bpress");
  strs.push_back("Si_avg_ifrac");
  strs.push_back("Si_i_anidf");
  strs.push_back("Si_i_anidr");
  strs.push_back("Si_i_avsdf");
  strs.push_back("Si_i_avsdr");
  strs.push_back("Si_i_bpress");
  strs.push_back("Si_i_ifrac");
  strs.push_back("Si_i_qref");
  strs.push_back("Si_i_snowh");
  strs.push_back("Si_i_tref");
  strs.push_back("Si_i_Tsfc");
  strs.push_back("Si_i_U10");
  strs.push_back("Si_o_bpress");
  strs.push_back("Si_o_ifrac");
  strs.push_back("Sl_a_anidf");
  strs.push_back("Sl_a_anidr");
  strs.push_back("Sl_a_avsdf");
  strs.push_back("Sl_a_avsdr");
  strs.push_back("Sl_a_fu");
  strs.push_back("Sl_a_fv");
  strs.push_back("Sl_a_qref");
  strs.push_back("Sl_a_ram1");
  strs.push_back("Sl_a_snowh");
  strs.push_back("Sl_a_soilw");
  strs.push_back("Sl_a_T");
  strs.push_back("Sl_a_tref");
  strs.push_back("Sl_a_U10");
  strs.push_back("Sl_l_anidf");
  strs.push_back("Sl_l_anidr");
  strs.push_back("Sl_l_avsdf");
  strs.push_back("Sl_l_avsdr");
  strs.push_back("Sl_l_fu");
  strs.push_back("Sl_l_fv");
  strs.push_back("Sl_l_qref");
  strs.push_back("Sl_l_ram1");
  strs.push_back("Sl_l_snowh");
  strs.push_back("Sl_l_soilw");
  strs.push_back("Sl_l_T");
  strs.push_back("Sl_l_tref");
  strs.push_back("Sl_l_U10");
  strs.push_back("So_a_anidf");
  strs.push_back("So_a_anidr");
  strs.push_back("So_a_avsdf");
  strs.push_back("So_a_avsdr");
  strs.push_back("So_a_duu10");
  strs.push_back("So_a_qref");
  strs.push_back("So_a_re");
  strs.push_back("So_a_ssq");
  strs.push_back("So_a_SST");
  strs.push_back("So_a_tref");
  strs.push_back("So_a_U10");
  strs.push_back("So_a_ustar");
  strs.push_back("So_i_bldepth");
  strs.push_back("So_i_dhdx");
  strs.push_back("So_i_dhdy");
  strs.push_back("So_i_fswpen");
  strs.push_back("So_i_SSS");
  strs.push_back("So_i_SST");
  strs.push_back("So_i_U");
  strs.push_back("So_i_V");
  strs.push_back("So_o_anidf");
  strs.push_back("So_o_anidr");
  strs.push_back("So_o_avsdf");
  strs.push_back("So_o_avsdr");
  strs.push_back("So_o_bldepth");
  strs.push_back("So_o_dhdx");
  strs.push_back("So_o_dhdy");
  strs.push_back("So_o_duu10");
  strs.push_back("So_o_fswpen");
  strs.push_back("So_o_qref");
  strs.push_back("So_o_re");
  strs.push_back("So_o_ssq");
  strs.push_back("So_o_SSS");
  strs.push_back("So_o_SST");
  strs.push_back("So_o_tref");
  strs.push_back("So_o_U");
  strs.push_back("So_o_U10");
  strs.push_back("So_o_ustar");
  strs.push_back("So_o_V");
  strs.push_back("Sx_a_anidf");
  strs.push_back("Sx_a_anidr");
  strs.push_back("Sx_a_avsdf");
  strs.push_back("Sx_a_avsdr");
  strs.push_back("Sx_a_ifrac");
  strs.push_back("Sx_a_lfrac");
  strs.push_back("Sx_a_ofrac");
  strs.push_back("Sx_a_qref");
  strs.push_back("Sx_a_Tref");
  strs.push_back("Sx_a_Tsfc");
  strs.push_back("Sx_a_U10");
  strs.push_back("");

  std::map<std::size_t, std::string> hashes;
  std::size_t num_collisions = 0;

  for (auto s : strs) {
    // compute hash of string s and check whether it's already in the list
    auto hash = flecsi::utils::string_hash<std::size_t>(s, s.size());
    if (hashes.count(hash) > 0) {
      printf(
          "COLLISION: '%s' collides with '%s'\n", s.c_str(),
          hashes.at(hash).c_str());
      ++num_collisions;
    } else {
      hashes.insert({hash, s});
    }
  }

  EXPECT_EQ(num_collisions, 0);

} // TEST

/*~-------------------------------------------------------------------------~-*
 * Formatting options
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~-------------------------------------------------------------------------~-*/
