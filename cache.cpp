#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cstring>
#include <fstream>
#include <ctime>
#include <math.h>

class CacheSimulator{
public:
  class set_value {
  public:
    long value;
    bool dirty;
    set_value () : dirty(false) {}
    set_value (long value, bool dirty) : value(value), dirty(dirty) {}
  };

  class Cache {
  public:
    std::vector <std::map <std::string, set_value> > set; //cache set
    std::vector <std::map <long, std::string> > age_map; //map for LRU
    
    Cache () {}
    ~Cache () {}
  };
  
  Cache insn_cache[2];
  Cache data_cache[2];
  std::set <std::string> addr_set;
  long cache_level;
  long hit_time[2];
  long cache_struct[2];
  long DRAM_time;
  long insn_capacity[2];
  long data_capacity[2];
  long insn_way_num[2]; //how many ways set associative, indicating associativity
  long data_way_num[2];
  long insn_block_size[2];
  long data_block_size[2];
  long insn_replace_algorithm[2];
  long data_replace_algorithm[2];
  long insn_alloc_on_miss[2];
  long data_alloc_on_miss[2];
  
  long main_access;
  long l1_access;
  long l2_access;
  long l2_write;
  
  long insn_set_num[2];
  long data_set_num[2];
  long sample_num[2];
  
  std::string operation;
  std::string input_addr;
  std::string address;
  std::string tag_bits[2];
  long set_index[2];
  
  long total[2];
  long insn_total[2];
  long data_total[2];
  long data_rtotal[2];
  long data_wtotal[2];
  long insn_hit[2];
  long data_hit[2];
  long data_rhit[2];
  long data_whit[2];
  long insn_miss[2];
  long data_miss[2];
  long data_rmiss[2];
  long data_wmiss[2];

  long insn_compulsory_miss[2];
  long data_rcompulsory_miss[2];
  long data_wcompulsory_miss[2];
  long insn_capacity_miss[2];
  long data_rcapacity_miss[2];
  long data_wcapacity_miss[2];
  long insn_conflict_miss[2];
  long data_rconflict_miss[2];
  long data_wconflict_miss[2];

  double total_time[2];
  double total_avg[2];
  double insn_time[2];
  double insn_avg[2];
  double data_time[2];
  double data_avg[2];
  double data_rtime[2];
  double data_ravg[2];
  double data_wtime[2];
  double data_wavg[2];

  
  CacheSimulator () : cache_level(1), main_access(0), l1_access(0), l2_access(0), l2_write(0) {
    for (size_t i = 0; i < 2; ++i) {
      total[i] = 0;
      insn_total[i] = 0;
      data_total[i] = 0;
      data_rtotal[i] = 0;
      data_wtotal[i] = 0;
      insn_hit[i] = 0;
      data_hit[i] = 0;
      data_rhit[i] = 0;
      data_whit[i] = 0;
      insn_miss[i] = 0;
      data_miss[i] = 0;
      data_rmiss[i] = 0;
      data_wmiss[i] = 0;
      insn_set_num[i] = 0;
      data_set_num[i] = 0;
      sample_num[i] = 0;
      insn_compulsory_miss[i] = 0;
      insn_capacity_miss[i] = 0;
      insn_conflict_miss[i] = 0;
      data_wcompulsory_miss[i] = 0;
      data_wcapacity_miss[i] = 0;
      data_wconflict_miss[i] = 0;
      data_rcompulsory_miss[i] = 0;
      data_rcapacity_miss[i] = 0;
      data_rconflict_miss[i] = 0;
      total_time[i] = 0;
      total_avg[i] = 0;
      insn_time[i] = 0;
      insn_avg[i] = 0;
      data_time[i] = 0;
      data_avg[i] = 0;
      data_rtime[i] = 0;
      data_ravg[i] = 0;
      data_wtime[i] = 0;
      data_wavg[i] = 0;
    }
  }

  ~CacheSimulator () {}


  void timing () {
    if (cache_level == 2) {
      //calculate second level average time first
      double l2_total = insn_total[1]+data_rtotal[1]+data_wtotal[1];
      double l2_miss = insn_miss[1]+data_rmiss[1]+data_wmiss[1];
      double l2_hit = l2_total - l2_miss;
      double l2_hit_time = l2_hit * (double)hit_time[1];
      double l2_miss_time = l2_miss * (double)DRAM_time;
      total_time[1] = l2_hit_time + l2_miss_time;
      total_avg[1] = (double)hit_time[1] + ((double)l2_miss/(double)l2_total) * (double)DRAM_time;
      
      double l1_total = insn_total[0]+data_rtotal[0]+data_wtotal[0];
      double l1_miss = insn_miss[0]+data_rmiss[0]+data_wmiss[0];
      double l1_hit = l1_total - l1_miss;
      double l1_hit_time = l1_hit * (double)hit_time[0];
      double l1_miss_time = total_time[1];
      total_time[0] = l1_hit_time + l1_miss_time;
      total_avg[0] = (double)hit_time[0] + ((double)l1_miss/(double)l1_total) * total_avg[1];
    }
    else {
      double l1_total = insn_total[0]+data_rtotal[0]+data_wtotal[0];
      double l1_miss = insn_miss[0]+data_rmiss[0]+data_wmiss[0];
      double l1_hit = l1_total - l1_miss;
      double l1_hit_time = l1_hit * (double)hit_time[0];
      double l1_miss_time = l1_miss * (double)DRAM_time;
      total_time[0] = l1_hit_time + l1_miss_time;
      total_avg[0] = (double)hit_time[0] + ((double)l1_miss/(double)l1_total) * (double)DRAM_time;
    }
  }



  void initialize (long * a) {
    std::string str;
    std::getline(std::cin,str);
    if (checkinput(str)) {
      if (!strcmp(str.data(),"")) {
        std::cout << "please enter a number!\n";
        initialize(a);
      }
      *a = atoi(str.data());
    }
    else {
      std::cout << "please enter a number! not some nonsense!\n";
      initialize(a);
    }
  }
  
  int checkinput (std::string str) {
    int i = 0;
    while (str[i] != '\0') {
      if ((str[i] < '0')||(str[i] > '9')) {
        return 0;
      }
      i++;
    }
    return 1;
  }
  
  void print (int i) {
//================================================================================================
    
    std::cout << "\n\nlevel : " << i+1 << std::endl;
    std::cout << "*********************************************************************\n";
    std::cout << "*********************************************************************\n";
    
    if (cache_struct[i] == 0) { //unified
      double total_miss_rate = (double)(insn_miss[i]+data_rmiss[i]+data_wmiss[i])/(double)(insn_total[i]+data_rtotal[i]+data_wtotal[i]);
      double insn_miss_rate = (double)insn_miss[i]/(double)insn_total[i];
      double data_wmiss_rate = (double)data_wmiss[i]/(double)data_wtotal[i];
      double data_rmiss_rate = (double)data_rmiss[i]/(double)data_rtotal[i];
      double data_miss_rate = (double)(data_wmiss[i]+data_rmiss[i])/(double)(data_wtotal[i]+data_rtotal[i]);
      
      std::cout << "\nunified cache\n-----------------------------------------------------------\n";
      
      std::cout << "\ntotal number:\t\t" << insn_total[i]+data_rtotal[i]+data_wtotal[i]-1 << std::endl;
      std::cout << "total hit:\t\t" << insn_total[i]+data_rtotal[i]+data_wtotal[i]-1-insn_miss[i]-data_rmiss[i]-data_wmiss[i] << std::endl;
      std::cout << "total miss:\t\t" << insn_miss[i]+data_rmiss[i]+data_wmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << insn_compulsory_miss[i]+data_rcompulsory_miss[i]+data_wcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << insn_capacity_miss[i]+data_rcapacity_miss[i]+data_wcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << insn_conflict_miss[i]+data_rconflict_miss[i]+data_wconflict_miss[i] << std::endl;
      std::cout << "total miss rate:\t" << total_miss_rate << std::endl;
      std::cout << "total access time:\t" << total_time[i] << std::endl;
      std::cout << "average access time:\t" << total_avg[i] << std::endl;


      std::cout << "\ninsn fetch total:\t" << insn_total[i] << std::endl;
      std::cout << "insn fetch hit:\t\t" << insn_total[i]-insn_miss[i] << std::endl;
      std::cout << "insn fetch miss:\t" << insn_miss[i] << std::endl;
      std::cout << "compulsory miss:\t" << insn_compulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << insn_capacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << insn_conflict_miss[i] << std::endl;
      std::cout << "insn miss rate:\t\t" << insn_miss_rate << std::endl;


      std::cout << "\ndata total:\t\t" << data_wtotal[i]+data_rtotal[i]-1 << std::endl;
      std::cout << "data hit:\t\t" << data_wtotal[i]+data_rtotal[i]-1-data_rmiss[i]-data_wmiss[i] << std::endl;
      std::cout << "data miss:\t\t" << data_rmiss[i]+data_wmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << data_rcompulsory_miss[i]+data_wcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << data_rcapacity_miss[i]+data_wcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << data_rconflict_miss[i]+data_wconflict_miss[i] << std::endl;
      std::cout << "data miss rate:\t\t" << data_miss_rate << std::endl;

      std::cout << "\ndata read total:\t" << data_rtotal[i]-1 << std::endl;
      std::cout << "data read hit:\t\t" << data_rtotal[i]-data_rmiss[i]-1 << std::endl;
      std::cout << "data read miss:\t\t" << data_rmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << data_rcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << data_rcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << data_rconflict_miss[i] << std::endl;
      std::cout << "data read miss rate:\t" << data_rmiss_rate << std::endl;
      
      std::cout << "\ndata write total:\t" << data_wtotal[i] << std::endl;
      std::cout << "data write hit:\t\t" << data_wtotal[i]-data_wmiss[i] << std::endl;
      std::cout << "data write miss:\t" << data_wmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << data_wcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << data_wcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << data_wconflict_miss[i] << std::endl;
      std::cout << "data write miss rate:\t" << data_wmiss_rate << std::endl;
    }
    else { //separate
      //double total_miss_rate = (insn_miss[i]+data_rmiss[i]+data_wmiss[i])/(insn_total[i]+data_rtotal[i]+data_wtotal[i]);
      double insn_miss_rate = (double)insn_miss[i]/(double)insn_total[i];
      double data_wmiss_rate = (double)data_wmiss[i]/(double)data_wtotal[i];
      double data_rmiss_rate = (double)data_rmiss[i]/(double)data_rtotal[i];
      double data_miss_rate = (double)(data_wmiss[i]+data_rmiss[i])/(double)(data_wtotal[i]+data_rtotal[i]);
      
      std::cout << "l1 instruction cache\n-----------------------------------------------------\n";
      std::cout << "total access time (for both insn and data):\t" << total_time[i] << std::endl;
      std::cout << "average access time (for both insn and data):\t" << total_avg[i] << std::endl;

      std::cout << "\ntotal number:\t\t" << insn_total[i]-1 << std::endl;
      std::cout << "total hit:\t\t" << insn_total[i]-insn_miss[i]-1 << std::endl;
      std::cout << "total miss:\t\t" << insn_miss[i] << std::endl;
      std::cout << "compulsory miss:\t" << insn_compulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << insn_capacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << insn_conflict_miss[i] << std::endl;
      std::cout << "total miss rate:\t" << insn_miss_rate << std::endl;

      std::cout << "\ninsn fetch total:\t" << insn_total[i]-1 << std::endl;
      std::cout << "insn fetch hit:\t\t" << insn_total[i]-insn_miss[i]-1 << std::endl;
      std::cout << "insn fetch miss:\t" << insn_miss[i] << std::endl;
      std::cout << "compulsory miss:\t" << insn_compulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << insn_capacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << insn_conflict_miss[i] << std::endl;
      std::cout << "insn miss rate:\t\t" << insn_miss_rate << std::endl;

      std::cout << "\ndata total:\t\t0\n";
      std::cout << "data hit:\t\t0\n";
      std::cout << "data miss:\t\t0\n";
      std::cout << "compulsory miss:\t0\n";
      std::cout << "capacity miss:\t\t0\n";
      std::cout << "conflict miss:\t\t0\n";
      std::cout << "data miss rate:\t\t0.0000000\n";

      std::cout << "\ndata read total:\t0\n";
      std::cout << "data read hit:\t\t0\n";
      std::cout << "data read miss:\t\t0\n";
      std::cout << "compulsory miss:\t0\n";
      std::cout << "capacity miss:\t\t0\n";
      std::cout << "conflict miss:\t\t0\n";
      std::cout << "data read miss rate:\t0.0000000\n";

      std::cout << "\ndata write total:\t0\n";
      std::cout << "data write hit:\t0\n";
      std::cout << "data write miss:\t0\n";
      std::cout << "compulsory miss:\t0\n";
      std::cout << "capacity miss:\t\t0\n";
      std::cout << "conflict miss:\t\t0\n";
      std::cout << "data write miss rate:\t0.0000000\n";


      
      std::cout << "\n\nl1 data cache\n-----------------------------------------------------\n";
      std::cout << "total access time (for both insn and data):\t" << total_time[i] << std::endl;
      std::cout << "average access time (for both insn and data):\t" << total_avg[i] << std::endl;
      std::cout << "\ntotal number:\t\t" << data_rtotal[i]+data_wtotal[i] << std::endl;
      std::cout << "total hit:\t\t" << data_rtotal[i]+data_wtotal[i]-data_rmiss[i]-data_wmiss[i] << std::endl;
      std::cout << "total miss:\t\t" << data_rmiss[i]+data_wmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << data_rcompulsory_miss[i]+data_wcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << data_rcapacity_miss[i]+data_wcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << data_rconflict_miss[i]+data_wconflict_miss[i] << std::endl;
      std::cout << "total miss rate:\t" << data_miss_rate << std::endl;

      std::cout << "\ninsn fetch total:\t0\n";
      std::cout << "insn fetch hit:\t0\n";
      std::cout << "insn fetch miss:\t0\n";
      std::cout << "compulsory miss:\t0\n";
      std::cout << "capacity miss:\t\t0\n";
      std::cout << "conflict miss:\t\t0\n";
      std::cout << "insn miss rate:\t\t0.0000000\n";

      std::cout << "\ndata total:\t\t" << data_wtotal[i]+data_rtotal[i] << std::endl;
      std::cout << "data hit:\t\t" << data_wtotal[i]+data_rtotal[i]-data_rmiss[i]-data_wmiss[i] << std::endl;
      std::cout << "data miss:\t\t" << data_rmiss[i]+data_wmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << data_rcompulsory_miss[i]+data_wcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << data_rcapacity_miss[i]+data_wcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << data_rconflict_miss[i]+data_wconflict_miss[i] << std::endl;
      std::cout << "data miss rate:\t\t" << data_miss_rate << std::endl;

      std::cout << "\ndata read total:\t" << data_rtotal[i] << std::endl;
      std::cout << "data read hit:\t\t" << data_rtotal[i]-data_rmiss[i] << std::endl;
      std::cout << "data read miss:\t\t" << data_rmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << data_rcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << data_rcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << data_rconflict_miss[i] << std::endl;
      std::cout << "data read miss rate:\t" << data_rmiss_rate << std::endl;
      
      std::cout << "\ndata write total:\t" << data_wtotal[i] << std::endl;
      std::cout << "data write hit:\t\t" << data_wtotal[i]-data_wmiss[i] << std::endl;
      std::cout << "data write miss:\t" << data_wmiss[i] << std::endl;
      std::cout << "compulsory miss:\t" << data_wcompulsory_miss[i] << std::endl;
      std::cout << "capacity miss:\t\t" << data_wcapacity_miss[i] << std::endl;
      std::cout << "conflict miss:\t\t" << data_wconflict_miss[i] << std::endl;
      std::cout << "data write miss rate:\t" << data_wmiss_rate << std::endl;
    }
  }
  


  void get_params () {
    std::cout << "please enter cache level (1 or 2)\n";
    initialize(&cache_level);
    std::cout << "please enter DRAM_time\n";
    initialize(&DRAM_time);
    std::cout << "please enter level 1 cache hit time\n";
    initialize(&hit_time[0]);
    std::cout << "please enter level 1 cache structure (0 for unified and 1 for separate cache)\n";
    initialize(&cache_struct[0]);
    if (cache_struct[0] != 0 && cache_struct[0] != 1) {
      std::cerr << "wrong input for level 1 cache structure\n";
      exit(EXIT_FAILURE);
    }
    if (cache_struct[0] == 0) { //unified level 1 cache
      std::cout << "please enter level 1 cache capacity in bytes (value should be a power of 2)\n";
      initialize(&insn_capacity[0]);
      std::cout << "please enter level 1 cache associativity (how many ways set associative, value should be a power of 2)\n";
      initialize(&insn_way_num[0]);
      std::cout << "please enter level 1 cache block_size in bytes (value should be a power of 2)\n";
      initialize(&insn_block_size[0]);
      std::cout << "please enter level 1 cache replacement algorithm (0 for LRU and 1 for RND)\n";
      initialize(&insn_replace_algorithm[0]);
      if (insn_replace_algorithm[0] != 0 && insn_replace_algorithm[0] != 1) {
        std::cerr << "wrong input for level 1 cache replacement algorithm\n";
        exit(EXIT_FAILURE);
      }
      std::cout << "please enter level 1 cache alloc_on_miss (1 for yes and 0 for no)\n";
      initialize(&insn_alloc_on_miss[0]);
    }
    else { //separate level 1 cache
      std::cout << "please enter level 1 insn cache capacity in bytes (value should be a power of 2)\n";
      initialize(&insn_capacity[0]);
      std::cout << "please enter level 1 data cache capacity in bytes (value should be a power of 2)\n";
      initialize(&data_capacity[0]);
      std::cout << "please enter level 1 insn cache associativity (how many ways set associative, value should be a power of 2)\n";
      initialize(&insn_way_num[0]);
      std::cout << "please enter level 1 data cache associativity (how many ways set associative, value should be a power of 2)\n";
      initialize(&data_way_num[0]);
      std::cout << "please enter level 1 insn cache block_size in bytes (value should be a power of 2)\n";
      initialize(&insn_block_size[0]);
      std::cout << "please enter level 1 data cache block_size in bytes (value should be a power of 2)\n";
      initialize(&data_block_size[0]);
      std::cout << "please enter level 1 insn cache replacement algorithm (0 for LRU and 1 for RND)\n";
      initialize(&insn_replace_algorithm[0]);
      if (insn_replace_algorithm[0] != 0 && insn_replace_algorithm[0] != 1) {
        std::cerr << "wrong input for level 1 insn cache replacement algorithm\n";
        exit(EXIT_FAILURE);
      }
      std::cout << "please enter level 1 data cache replacement algorithm (0 for LRU and 1 for RND)\n";
      initialize(&data_replace_algorithm[0]);
      if (data_replace_algorithm[0] != 0 && data_replace_algorithm[0] != 1) {
        std::cerr << "wrong input for level 1 insn cache replacement algorithm\n";
        exit(EXIT_FAILURE);
      }
      std::cout << "please enter level 1 insn cache alloc_on_miss (1 for yes and 0 for no)\n";
      initialize(&insn_alloc_on_miss[0]);
      std::cout << "please enter level 1 data cache alloc_on_miss (1 for yes and 0 for no)\n";
      initialize(&data_alloc_on_miss[0]);
    }
//-----------------------------------------------------------------------------------------------
    if (cache_level == 2) { //level 2 cache
      std::cout << "please enter level 2 cache hit time\n";
      initialize(&hit_time[1]);
      std::cout << "please enter level 2 cache structure (0 for unified and 1 for separate cache)\n";
      initialize(&cache_struct[1]);
      if (cache_struct[1] != 0 && cache_struct[1] != 1) {
        std::cerr << "wrong input for level 2 cache structure\n";
        exit(EXIT_FAILURE);
      }
      if (cache_struct[1] == 0) { //unified level 2 cache
        std::cout << "please enter level 2 cache capacity in bytes (value should be a power of 2)\n";
        initialize(&insn_capacity[1]);
        std::cout << "please enter level 2 cache associativity (how many ways set associative, value should be a power of 2)\n";
        initialize(&insn_way_num[1]);
        std::cout << "please enter level 2 cache block_size in bytes (value should be a power of 2)\n";
        initialize(&insn_block_size[1]);
        std::cout << "please enter level 2 cache replacement algorithm (0 for LRU and 1 for RND)\n";
        initialize(&insn_replace_algorithm[1]);
        if (insn_replace_algorithm[1] != 0 && insn_replace_algorithm[1] != 1) {
          std::cerr << "wrong input for level 2 cache replacement algorithm\n";
          exit(EXIT_FAILURE);
        }
        std::cout << "please enter level 2 cache alloc_on_miss (1 for yes and 0 for no)\n";
        initialize(&insn_alloc_on_miss[1]);
      }
      else { //separate level 2 cache
        std::cout << "please enter level 2 insn cache capacity in bytes (value should be a power of 2)\n";
        initialize(&insn_capacity[1]);
        std::cout << "please enter level 2 data cache capacity in bytes (value should be a power of 2)\n";
        initialize(&data_capacity[1]);
        std::cout << "please enter level 2 insn cache associativity (how many ways set associative, value should be a power of 2)\n";
        initialize(&insn_way_num[1]);
        std::cout << "please enter level 2 data cache associativity (how many ways set associative, value should be a power of 2)\n";
        initialize(&data_way_num[1]);
        std::cout << "please enter level 2 insn cache block_size in bytes (value should be a power of 2)\n";
        initialize(&insn_block_size[1]);
        std::cout << "please enter level 2 data cache block_size in bytes (value should be a power of 2)\n";
        initialize(&data_block_size[1]);
        std::cout << "please enter level 2 insn cache replacement algorithm (0 for LRU and 1 for RND)\n";
        initialize(&insn_replace_algorithm[1]);
        if (insn_replace_algorithm[1] != 0 && insn_replace_algorithm[1] != 1) {
          std::cerr << "wrong input for level 2 insn cache replacement algorithm\n";
          exit(EXIT_FAILURE);
        }
        std::cout << "please enter level 2 data cache replacement algorithm (0 for LRU and 1 for RND)\n";
        initialize(&data_replace_algorithm[1]);
        if (data_replace_algorithm[1] != 0 && data_replace_algorithm[1] != 1) {
          std::cerr << "wrong input for level 2 insn cache replacement algorithm\n";
          exit(EXIT_FAILURE);
        }
        std::cout << "please enter level 2 insn cache alloc_on_miss (1 for yes and 0 for no)\n";
        initialize(&insn_alloc_on_miss[1]);
        std::cout << "please enter level 2 data cache alloc_on_miss (1 for yes and 0 for no)\n";
        initialize(&data_alloc_on_miss[1]);
      }
    }
  }
  
  void parse_address () {
    //32 bits address, 8 hex digits
    size_t size = 8 - input_addr.size();
    for (size_t i = 0; i < size; ++i) {
      //fill will '0' for empty digits
      address += "0000";
    }
    for (size_t i = 0; i < input_addr.size(); ++i) {
      if (input_addr[i] == '0') {
        address += "0000";
      }
      else if (input_addr[i] == '1') {
        address += "0001";
      }
      else if (input_addr[i] == '2') {
        address += "0010";
      }
      else if (input_addr[i] == '3') {
        address += "0011";
      }
      else if (input_addr[i] == '4') {
        address += "0100";
      }
      else if (input_addr[i] == '5') {
        address += "0101";
      }
      else if (input_addr[i] == '6') {
        address += "0110";
      }
      else if (input_addr[i] == '7') {
        address += "0111";
      }
      else if (input_addr[i] == '8') {
        address += "1000";
      }
      else if (input_addr[i] == '9') {
        address += "1001";
      }
      else if (input_addr[i] == 'a') {
        address += "1010";
      }
      else if (input_addr[i] == 'b') {
        address += "1011";
      }
      else if (input_addr[i] == 'c') {
        address += "1100";
      }
      else if (input_addr[i] == 'd') {
        address += "1101";
      }
      else if (input_addr[i] == 'e') {
        address += "1110";
      }
      else if (input_addr[i] == 'f') {
        address += "1111";
      }
      else {
        std::cerr << "input file format error!" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }
  
  void split_addr (int j) {
    int set_bits_num = 0;
    int offset_bits_num = 0;
    int tag_bits_num = 0;
    int set_digit = 0;
    
    if (cache_struct[j] == 0) { //unified
      set_bits_num = log10(insn_set_num[j])/log10(2);
      offset_bits_num = log10(insn_block_size[j])/log10(2);
      tag_bits_num = address.size() - set_bits_num - offset_bits_num;
    }
    else { //data cache and insn cache separate
      if (operation == "0" || operation == "1") { //data read or write
        set_bits_num = log10(data_set_num[j])/log10(2);
        offset_bits_num = log10(data_block_size[j])/log10(2);
        tag_bits_num = address.size() - set_bits_num - offset_bits_num;
      }
      else if (operation == "2") { //instruction fetch
        set_bits_num = log10(insn_set_num[j])/log10(2);
        offset_bits_num = log10(insn_block_size[j])/log10(2);
        tag_bits_num = address.size() - set_bits_num - offset_bits_num;
      }
    }
    //split tag bits
    for (int i = 0; i < tag_bits_num; ++i) {
      tag_bits[j].push_back(address[i]);
    }
    std::size_t found = tag_bits[j].find('1');
    if (found == std::string::npos) {
      tag_bits[j] = "0";
    }
    else {
      tag_bits[j] = tag_bits[j].substr(found);
    }

    //split set index bits
    int n = 1;
    for (int i = tag_bits_num; i < tag_bits_num + set_bits_num; ++i) {
      set_digit = address[i] - 48; //convert to number
      set_index[j] += set_digit * pow(2, (set_bits_num - n));
      ++n;
    }
  }
  
  void access_main () {
    ++main_access;
  }

  void write_back (int i, std::string tag_bits) {
    ++total[i];
    ++data_wtotal[i];
    size_t index = set_index[i];
    std::map<std::string, set_value>::iterator it = insn_cache[i].set[index].find(tag_bits);
    if (it != insn_cache[i].set[index].end()) {
      //update block so set dirty bit
      it->second.dirty = true;
      ++data_whit[i];
      if (insn_replace_algorithm[i] == 0) { //update age_map
        long old_age = it->second.value;
        std::string tag_to_update = insn_cache[i].age_map[index].find(old_age)->second;
        insn_cache[i].age_map[index].erase(old_age);
        insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_to_update));
        it->second.value = total[i];
      }
    }

    //address is new, miss
    else {
      //compulsory miss
      if (insn_cache[i].set[index].size() < (unsigned long) insn_way_num[i]) {
        if (insn_alloc_on_miss[i] == 1) { //write on miss, update all levels
          access_main(); //first write to main
          set_value new_set_value(total[i], true); //dirty
          insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits, new_set_value));
          //write, update age map
          if (insn_replace_algorithm[i] == 0) {
            insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits));
          }
        }
        else { //no write on miss, so write to level 2 or main without reading after
          access_main();
        }
      }
      //capacity miss
      else {
        if (insn_alloc_on_miss[i] == 1) {
          access_main(); //first write to main
          if (insn_replace_algorithm[i] == 0) { //LRU
            std::string tag_to_erase = insn_cache[i].age_map[index].begin()->second;
            //std::cout << tag_to_erase << " <- write back: tag_to_erase\n";
            if (i == 1 && insn_cache[i].set[index][tag_to_erase].dirty == true) {
              access_main();
            }
            insn_cache[i].age_map[index].erase(insn_cache[i].age_map[index].begin());
            insn_cache[i].set[index].erase(tag_to_erase);
            set_value new_set_value(total[i], true); //the block is dirty
            insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits, new_set_value));
            insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits));
          }
          else if (insn_replace_algorithm[i] == 1) { //RND
            int rand_num = rand() % insn_way_num[i];
            std::map<std::string, set_value>::iterator temp_it = insn_cache[i].set[index].begin();
            for (int n = 0; n < rand_num; ++n) {
              ++temp_it;
            }
            if (i == 0 && temp_it->second.dirty == true && cache_level == 2) {
              write_back(1, temp_it->first);
            }
            else if (i == 1 && temp_it->second.dirty == true) {
              access_main();
            }
            insn_cache[i].set[index].erase(temp_it);
            set_value new_set_value(total[i], true); //the block is dirty
            insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits, new_set_value));
          }
        }
        //no write on miss, write to the next level directly
        else {
          if (i == 0) {
            if (cache_level == 2) {
              access_cache(1);
            }
          }
          else {
            access_main();
          }
        }
      }
    }
  }
  
  void access_cache (size_t i) {
    std::map<std::string, set_value>::iterator it;
    size_t index = set_index[i];
//=========================================== UNIFIED =============================================
    if (cache_struct[i] == 0) { //unified L1 cache
      ++total[i];
      ++sample_num[i];
// ==========================================DATA READ ============================================
      if (operation == "0" || operation == "2") { //data read or insn fetch
        if (operation == "0") {
          ++data_rtotal[i];
        }
        else {
          ++insn_total[i];
        }
        it = insn_cache[i].set[index].find(tag_bits[i]);
        //if tag is matched
        if (it != insn_cache[i].set[index].end()) {
          if (operation == "0") {
            ++data_rhit[i];
          }
          else {
            ++insn_hit[i];
          }
          //hit, update age
          if (insn_replace_algorithm[i] == 0) {
            long old_age = it->second.value;
            std::string tag_to_update = insn_cache[i].age_map[index].find(old_age)->second;
            insn_cache[i].age_map[index].erase(old_age);
            insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_to_update));
            it->second.value = total[i];
          }
        }


        //address is new, miss
        else {
          if (operation == "0") { //data read miss
            ++data_rmiss[i];
          }
          else { //insn fetch miss
            ++insn_miss[i];
          }
          //compulsory miss
          if (insn_cache[i].set[index].size() < (unsigned long) insn_way_num[i]) {
            //if (i == 0) {
              if (operation == "0") {
                ++insn_compulsory_miss[i];
              }
              else {
                ++data_rcompulsory_miss[i];
              }
            //}
            //first, search in level 2
            if (i == 0) { //if level 1 cache
              if (cache_level == 2) {
                access_cache(1);
              }
              // add tag after l2
              set_value new_set_value(total[i], false);
              insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              if (insn_replace_algorithm[i] == 0) {
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
            //if already level 2, go for main memory, add tag
            else {
              access_main(); //if missed, go for main memory
              set_value new_set_value(total[i], false);
              insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              if (insn_replace_algorithm[i] == 0) {
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));      
              }
            }
          }
          //capacity miss, evict some blocks
          else {
            //check if cache is full
            if (1) {
              bool not_full = false;
              size_t size = insn_cache[i].set.size();
              if (size == 0) {
                not_full = true;
              }
              else { //check every set
                for (size_t j = 0; j < size; ++j) {
                  if (insn_cache[i].set[j].size() != (unsigned) insn_way_num[i]) {
                    //there exists a set that is not full
                    not_full = true;
                  }
                }
              }
              if (not_full == true) {
                if (operation == "0") {
                  ++insn_conflict_miss[i];
                }
                else {
                  ++data_rconflict_miss[i];
                }
              }
              else {
                if (addr_set.find(tag_bits[i]) == addr_set.end()) {
                  //new adrees, compulsory miss
                  if (operation == "0") {
                    ++insn_compulsory_miss[i];
                  }
                  else {
                    ++data_rcompulsory_miss[i];
                  }
                }
                else {
                  if (operation == "0") {
                    ++insn_capacity_miss[i];
                  }
                  else {
                    ++data_rcapacity_miss[i];
                  }
                }
              }
            }
            //else, replace value using replace_algorithm
            if (insn_replace_algorithm[i] == 0) { //LRU
              //first, check if level 2 has the block
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
                std::string tag_to_erase = insn_cache[i].age_map[index].begin()->second;
                //if the block to be replaced is true, write to l2
                if (insn_cache[i].set[index][tag_to_erase].dirty == true && cache_level == 2) {
                  write_back(1, tag_to_erase); //remember to update age map of new added element
                }
                insn_cache[i].age_map[index].erase(insn_cache[i].age_map[index].begin());
                insn_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                if (insn_replace_algorithm[i] == 0) {
                  insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
                }
              }
              else { //already in level 2, add tag
                access_main();
                std::string tag_to_erase = insn_cache[i].age_map[index].begin()->second;
                insn_cache[i].age_map[index].erase(insn_cache[i].age_map[index].begin());
                //if the block to be replaced is true, write to main memory
                if (insn_cache[i].set[index][tag_to_erase].dirty == true) {
                  access_main();
                }
                insn_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
            else if (insn_replace_algorithm[i] == 1) { //RND
              if (i == 0) { //check level 2
                if (cache_level == 2) {
                  access_cache(1);
                }
                int rand_num = rand() % insn_way_num[i];
                std::map<std::string, set_value>::iterator temp_it = insn_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_it;
                }
                //dirty block from level 1 should go to level 2
                if (temp_it->second.dirty == true && cache_level == 2) {
                  write_back(1, temp_it->first);
                }
                insn_cache[i].set[index].erase(temp_it);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
              else { //already in level 2
                access_main();
                int rand_num = rand() % insn_way_num[i];
                std::map<std::string, set_value>::iterator temp_it = insn_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_it;
                }
                //dirty block from level 2 should go to main memory
                if (temp_it->second.dirty == true) {
                  access_main();
                }
                insn_cache[i].set[index].erase(temp_it);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
            }
          }
        }
      }

//======================================= DATA WRITE ==============================================
      
      else { //data write
        if (i == 0) {
          ++data_wtotal[i];
        }
        it = insn_cache[i].set[index].find(tag_bits[i]);
        //if tag is matched, write back, only write this level
        if (it != insn_cache[i].set[index].end()) {
          //update block so set dirty bit
          it->second.dirty = true;
          ++data_whit[i];
          if (insn_replace_algorithm[i] == 0) { //update age_map
            long old_age = it->second.value;
            std::string tag_to_update = insn_cache[i].age_map[index].find(old_age)->second;
            insn_cache[i].age_map[index].erase(old_age);
            insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_to_update));
            it->second.value = total[i];
          }
        }

        //address is new, miss
        else {
            ++data_wmiss[i];
          //compulsory miss
          if (insn_cache[i].set[index].size() < (unsigned long) insn_way_num[i]) {

            ++data_wcompulsory_miss[i];

            if (insn_alloc_on_miss[i] == 1) { //write on miss, update all levels
              if (i == 0) { //if level 1
                if (cache_level == 2) {
                  access_cache(1); //first write to level 2
                  ++data_rtotal[1]; //then read from level 2
                }
              }
              else { //if level 2
                access_main(); //first write to main
              }
              set_value new_set_value(total[i], true); //dirty
              insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              //write, update age map
              if (insn_replace_algorithm[i] == 0) {
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
            else { //no write on miss, so write to level 2 or main without reading after
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
              }
              else {
                access_main();
              }
            }
          }
          //capacity miss
          else {
            //check if cache is full
            if (1) {
              bool not_full = false;
              size_t size = insn_cache[i].set.size();
              if (size == 0) {
                not_full = true;
              }
              else { //check every set 
                for (size_t j = 0; j < size; ++j) {
                  if (insn_cache[i].set[j].size() != (unsigned) insn_way_num[i]) {
                    //there exists a set that is not full
                    not_full = true;
                  }
                }
              }
              if (not_full == true) {
                ++data_wconflict_miss[i];
              }
              else {
                if (addr_set.find(tag_bits[i]) == addr_set.end()) {
                  //new adrees, compulsory miss
                  ++data_wcompulsory_miss[i];
                }
                else {
                  ++data_wcapacity_miss[i];
                }
              }
            }


            if (insn_alloc_on_miss[i] == 1) {
              if (i == 0) { //if level 1
                if (cache_level == 2) {
                  access_cache(1); //first write to level 2
                  ++data_rtotal[1];
                }
              }
              else { //if level 2
                access_main(); //first write to main
              }
              if (insn_replace_algorithm[i] == 0) { //LRU
                std::string tag_to_erase = insn_cache[i].age_map[index].begin()->second;
                if (i == 0 && insn_cache[i].set[index][tag_to_erase].dirty == true && cache_level == 2) {
                  write_back(1, tag_to_erase); //remember to update age
                }
                else if (i == 1 && insn_cache[i].set[index][tag_to_erase].dirty == true) {
                  access_main();
                }
                insn_cache[i].age_map[index].erase(insn_cache[i].age_map[index].begin());
                insn_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], true); //the block is dirty
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
              else if (insn_replace_algorithm[i] == 1) { //RND
                int rand_num = rand() % insn_way_num[i];
                std::map<std::string, set_value>::iterator temp_it = insn_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_it;
                }
                if (i == 0 && temp_it->second.dirty == true && cache_level == 2) {
                  write_back(1, temp_it->first);
                }
                else if (i == 1 && temp_it->second.dirty == true) {
                  access_main();
                }
                insn_cache[i].set[index].erase(temp_it);
                set_value new_set_value(total[i], true); //the block is dirty
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
            }
            //no write on miss, write to the next level directly
            else {
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
              }
              else {
                access_main();
              }
            }
          }
        }
      }
    }


//separate cache================================================================================
    else {
      ++total[i];
      ++sample_num[i];
      if (operation == "0") { //data read
        ++data_rtotal[i];
        it = data_cache[i].set[index].find(tag_bits[i]);
        //if tag is matched, hit
        if (it != data_cache[i].set[index].end()) {
          ++data_rhit[i];
          if (data_replace_algorithm[i] == 0) {
            long old_age = it->second.value;
            std::string tag_to_update = data_cache[i].age_map[index].find(old_age)->second;
            data_cache[i].age_map[index].erase(old_age);
            data_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_to_update));
            it->second.value = total[i];
          }
        }

        //address is new, miss
        else {
          ++data_rmiss[i];
          //compulsory miss
          if (data_cache[i].set[index].size() < (unsigned long) data_way_num[i]) {
            ++data_rcompulsory_miss[i];
            if (i == 0) { //find in level 2 first
              if (cache_level == 2) {
                access_cache(1);
              }
            }
            else { //already level 2
              access_main();
            }
            set_value new_set_value(total[i], false);
            data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
            if (data_replace_algorithm[i] == 0) {
              data_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
            }
          }
          else {
            //check if cache is full
            if (1) {
              bool not_full = false;
              size_t size = data_cache[i].set.size();
              if (size == 0) {
                not_full = true;
              }
              else { //check every set 
                for (size_t j = 0; j < size; ++j) {
                  if (data_cache[i].set[j].size() != (unsigned) data_way_num[i]) {
                    //there exists a set that is not full
                    not_full = true;
                  }
                }
              }
              if (not_full == true) {
                ++data_rconflict_miss[i];
              }
              else {
                if (addr_set.find(tag_bits[i]) == addr_set.end()) {
                  //new adrees, compulsory miss
                  ++data_rcompulsory_miss[i];
                }
                else {
                  ++data_rcapacity_miss[i];
                }
              }
            }

            //else, replace value on miss with alorithm indicated by replace_algorithm
            if (data_replace_algorithm[i] == 0) { //LRU
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
                std::string tag_to_erase = data_cache[i].age_map[index].begin()->second;
                //if the block to be replaced is true, write to l2
                if (data_cache[i].set[index][tag_to_erase].dirty == true && cache_level == 2) {
                  write_back(1, tag_to_erase);
                }
                data_cache[i].age_map[index].erase(data_cache[i].age_map[index].begin());
                data_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], false);
                data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                
                data_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
              else {
                access_main();
                std::string tag_to_erase = data_cache[i].age_map[index].begin()->second;
                data_cache[i].age_map[index].erase(data_cache[i].age_map[index].begin());
                //if the block to be replaced is true, write to main memory
                if (data_cache[i].set[index][tag_to_erase].dirty == true) {
                  access_main();
                }
                data_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], false);
                data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                data_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
            else if (data_replace_algorithm[i] == 1) { //RND
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
                int rand_num = rand() % data_way_num[i];
                std::map<std::string, set_value>::iterator temp_temp_it = data_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_temp_it;
                }
                data_cache[i].set[index].erase(temp_temp_it);
                set_value new_set_value(total[i], false);
                data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
              else {
                access_main();
                int rand_num = rand() % data_way_num[i];
                std::map<std::string, set_value>::iterator temp_it = data_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_it;
                }
                //dirty block from level 2 should go to main memory
                if (temp_it->second.dirty == true) {
                  access_main();
                }
                data_cache[i].set[index].erase(temp_it);
                set_value new_set_value(total[i], false);
                data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
            }
          }
        }
      }
      else if (operation == "1") { //data write
        if (i == 0) {
          ++data_wtotal[i];
        }
        it = data_cache[i].set[index].find(tag_bits[i]);
        //if tag is matched
        if (it != data_cache[i].set[index].end()) {
          //update block so set dirty bit
          it->second.dirty = true;
          ++data_whit[i];
          if (data_replace_algorithm[i] == 0) { //LRU, update age_map
            long old_age = it->second.value;
            std::string tag_to_update = data_cache[i].age_map[index].find(old_age)->second;
            data_cache[i].age_map[index].erase(old_age);
            data_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_to_update));
            it->second.value = total[i];
          }
        }
        //address is new, miss
        else {
          //if (i == 0) {
            ++data_wmiss[i];
          //}
          //compulsory miss
          if (data_cache[i].set[index].size() < (unsigned long) data_way_num[i]) {
            //if (i == 0) {
              ++data_wcompulsory_miss[i];
            //}
            if (data_alloc_on_miss[i] == 1) { //if allocate on write miss
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1); //first write to level 2
                  ++data_rtotal[1]; //then read from level 2
                }
              }
              else { //if level 2
                access_main(); //first write to main
              }
              set_value new_set_value(total[i], true); //the block is dirty
              data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              if (data_replace_algorithm[i] == 0) {
                data_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
            else { //no write on miss
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
              }
              else {
                access_main();
              }
            }
          }
          else {
            //check if cache is full
            if (1) {
              bool not_full = false;
              size_t size = data_cache[i].set.size();
              if (size == 0) {
                not_full = true;
              }
              else { //check every set 
                for (size_t j = 0; j < size; ++j) {
                  if (data_cache[i].set[j].size() != (unsigned) data_way_num[i]) {
                    //there exists a set that is not full
                    not_full = true;
                  }
                }
              }
              if (not_full == true) {
                ++data_wconflict_miss[i];
              }
              else {
                if (addr_set.find(tag_bits[i]) == addr_set.end()) {
                  //new adrees, compulsory miss
                  ++data_wcompulsory_miss[i];
                }
                else {
                  ++data_wcapacity_miss[i];
                }
              }
            }
            //else, replace value on miss with alorithm indicated by replace_algorithm
            if (data_alloc_on_miss[i] == 1) {
              if (i == 0) { //if level 1
                if (cache_level == 2) {
                  access_cache(1); //first write to level 2
                  ++data_rtotal[1];
                }
              }
              else { //if level 2
                access_main(); //first write to main
              }
              if (data_replace_algorithm[i] == 0) { //LRU
                std::string tag_to_erase = data_cache[i].age_map[index].begin()->second;
                if (i == 0 && data_cache[i].set[index][tag_to_erase].dirty == true && cache_level == 2) {
                  write_back(1, tag_to_erase); //remember to update age
                }
                else if (i == 1 && data_cache[i].set[index][tag_to_erase].dirty == true) {
                  access_main();
                }
                data_cache[i].age_map[index].erase(data_cache[i].age_map[index].begin());
                data_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], true); //the block is dirty
                data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                data_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
              else if (data_replace_algorithm[i] == 1) { //RND
                int rand_num = rand() % data_way_num[i];
                std::map<std::string, set_value>::iterator temp_it = data_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_it;
                }
                if (i == 0 && temp_it->second.dirty == true && cache_level == 2) {
                  write_back(1, temp_it->first);
                }
                else if (i == 1 && temp_it->second.dirty == true) {
                  access_main();
                }
                data_cache[i].set[index].erase(temp_it);
                set_value new_set_value(total[i], true); //dirty
                data_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
            }
            else { //no write on miss
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
              }
              else {
                access_main();
              }
            }
          }
        }
      }


      else { //instruction fetch
        ++insn_total[i];
        it = insn_cache[i].set[index].find(tag_bits[i]);
        //if tag is matched
        if (it != insn_cache[i].set[index].end()) {
          ++insn_hit[i];
          if (insn_replace_algorithm[i] == 0) {
            long old_age = it->second.value;
            std::string tag_to_update = insn_cache[i].age_map[index].find(old_age)->second;
            insn_cache[i].age_map[index].erase(old_age);
            insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_to_update));
            it->second.value = total[i];
          }
        }
        //address is new
        else {
          ++insn_miss[i];
          if (insn_cache[i].set[index].size() < (unsigned long) insn_way_num[i]) {
            ++insn_compulsory_miss[i];
            if (i == 0) {
              if (cache_level == 2) {
                access_cache(1);
              }
              set_value new_set_value(total[i], false);
              insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              if (insn_replace_algorithm[i] == 0) {
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
            else {
              access_main(); //if missed, go for main memory
              set_value new_set_value(total[i], false);
              insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              if (insn_replace_algorithm[i] == 0) {
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
          }
          else {
            //check if cache is full
            if (1) {
              bool not_full = false;
              size_t size = insn_cache[i].set.size();
              if (size == 0) {
                not_full = true;
              }
              else { //check every set 
                for (size_t j = 0; j < size; ++j) {
                  if (insn_cache[i].set[j].size() != (unsigned) insn_way_num[i]) {
                    //there exists a set that is not full
                    not_full = true;
                  }
                }
              }
              if (not_full == true) {
                ++insn_conflict_miss[i];
              }
              else {
                if (addr_set.find(tag_bits[i]) == addr_set.end()) {
                  //new adrees, compulsory miss
                  ++insn_compulsory_miss[i];
                }
                else {
                  ++insn_capacity_miss[i];
                }
              }
            }
            //else, replace value on miss with alorithm indicated by replace_algorithm
            if (insn_replace_algorithm[i] == 0) { //LRU
              if (i == 0) {
                if (cache_level == 2) {
                  access_cache(1);
                }
                std::string tag_to_erase = insn_cache[i].age_map[index].begin()->second;
                insn_cache[i].age_map[index].erase(insn_cache[i].age_map[index].begin());
                insn_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
              else { //already level 2
                access_main();
                std::string tag_to_erase = insn_cache[i].age_map[index].begin()->second;
                insn_cache[i].age_map[index].erase(insn_cache[i].age_map[index].begin());
                //if the block to be replaced is true, write to main memory
                if (insn_cache[i].set[index][tag_to_erase].dirty == true) {
                  access_main();
                }
                insn_cache[i].set[index].erase(tag_to_erase);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
                insn_cache[i].age_map[index].insert(std::pair <long, std::string> (total[i], tag_bits[i]));
              }
            }
            else if (insn_replace_algorithm[i] == 1) { //RND
              if (i == 0) {
                int rand_num = rand() % insn_way_num[i];
                std::map<std::string, set_value>::iterator temp_temp_it = insn_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_temp_it;
                }
                insn_cache[i].set[index].erase(temp_temp_it);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
              else {
                access_main();
                int rand_num = rand() % insn_way_num[i];
                std::map<std::string, set_value>::iterator temp_it = insn_cache[i].set[index].begin();
                for (int n = 0; n < rand_num; ++n) {
                  ++temp_it;
                }
                //dirty block from level 2 should go to main memory
                if (temp_it->second.dirty == true) {
                  access_main();
                }
                insn_cache[i].set[index].erase(temp_it);
                set_value new_set_value(total[i], false);
                insn_cache[i].set[index].insert(std::pair <std::string, set_value> (tag_bits[i], new_set_value));
              }
            }
          }
        }
      }
    }
    //update address
    addr_set.insert(tag_bits[i]);
  }
  
  void set_cache (int i) {
    if (cache_struct[i] == 0) { //unified cache
      int block_num = insn_capacity[i]/insn_block_size[i];
      insn_set_num[i] = block_num/insn_way_num[i];
      insn_cache[i].set.resize(insn_set_num[i]);
      insn_cache[i].age_map.resize(insn_set_num[i]);
    }
    else { //insn and data cache separated
      int block_num = insn_capacity[i]/insn_block_size[i];
      insn_set_num[i] = block_num/insn_way_num[i];
      block_num = data_capacity[i]/data_block_size[i];
      data_set_num[i] = block_num/data_way_num[i];
      insn_cache[i].set.resize(insn_set_num[i]);
      data_cache[i].set.resize(data_set_num[i]);
      insn_cache[i].age_map.resize(insn_set_num[i]);
      data_cache[i].age_map.resize(data_set_num[i]);
    }
  }
};



int main (int argc, char ** argv) {
  //set random seed
  srand((int)time(0));

  if (argc != 2) {
    std::cerr << "wrong arguments!" << std::endl;
    return EXIT_FAILURE;
  }
  
  CacheSimulator simulator;
  simulator.get_params();
  simulator.set_cache(0);
  if (simulator.cache_level == 2) {
    simulator.set_cache(1);
  }
  std::ifstream myfile(argv[1]);
  
  if(!myfile.is_open()){
    std::cout << "fails to open file: " << argv[1] << std::endl;
    exit(EXIT_FAILURE);
  }
  
  std::cout << "\nsimulating...\n\n";
  std::string str;
  std::stringstream ss;
  ss << myfile.rdbuf();
  
  time_t beginning = time(NULL);

  while (!ss.eof()) {
    simulator.address.clear();
    simulator.tag_bits[0].clear();
    simulator.tag_bits[1].clear();
    simulator.set_index[0] = 0;
    simulator.set_index[1] = 0;
    
    ss >> simulator.operation;
    ss >> simulator.input_addr;
    
    simulator.parse_address();
    simulator.split_addr(0);

    if (simulator.cache_level == 2) {
      simulator.split_addr(1);
    }
    simulator.access_cache(0);
  }

  time_t end = time(NULL);
  double operation_time = difftime(end, beginning);

  simulator.timing();
  simulator.print(0);
  if (simulator.cache_level == 2) {
    simulator.print(1);
  }

  std::cout << "\nsimulation complete! simulation spent " << operation_time << " seconds\n";
  std::cout << "\nsimulation is run on Ubuntu 14 64 bit system (Virtual Machine)\n";
   
  myfile.close();
  return EXIT_SUCCESS;
}











