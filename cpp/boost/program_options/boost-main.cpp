/*
 * $Id$
 * 
 * Copyright (c) 2015-2018, Luca Fulchir<luker@fenrirproject.org>,
 * All rights reserved.
 *
 * This file is part of "libRaptorQ".
 *
 * libRaptorQ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * libRaptorQ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and a copy of the GNU Lesser General Public License
 * along with libRaptorQ.  If not, see <http://www.gnu.org/licenses/>.
 */

#include ".git.h"

#ifndef VERSION
#define VERSION "-"
#endif

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
namespace po=boost::program_options;


#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <cstdlib>
#include <vector>


void train(const po::variables_map& vm)
{
  std::string input_path, info_path, output_path;
  if (vm.count("input"))
    input_path = vm["input"].as<std::string>();
  if(vm.count("info"))
    info_path = vm["info"].as<std::string>();
  if(vm.count("output"))
    output_path = vm["output"].as<std::string>();
  std::cout<<"train: "<<input_path<<" "<<info_path<<" "<<output_path<<std::endl;
}

void recognize(const po::variables_map& vm)
{
  std::string params_path, output_path;
  std::vector<std::string> input_files;
  if (vm.count("input"))
    input_files = vm["input"].as<std::vector<std::string> >();
  if(vm.count("params"))
    params_path = vm["params"].as<std::string>();
  if(vm.count("output"))
    output_path = vm["output"].as<std::string>();
  std::cout<<"recognize: "<<boost::algorithm::join(input_files, " ")<<" "<<params_path<<" "<<output_path<<std::endl;
}

void score(const po::variables_map& vm)
{
  std::string test_path, output_path;
  std::string ethanol_files;
  if (vm.count("ethanol"))
    ethanol_files = vm["ethanol"].as<std::string>();
  if(vm.count("test"))
    test_path = vm["test"].as<std::string>();
  if(vm.count("output"))
    output_path = vm["output"].as<std::string>();
  std::cout<<"test: "<<ethanol_files<<" "<<test_path<<" "<<output_path<<std::endl;
}

//-------------------------------------------------------------------------
int main(int ac, char* av[])
{
	std::cerr << "Hello boost " VERSION " Compiled: " __DATE__ " " __TIME__ << std::endl;
	
  po::options_description desc("General options");
  std::string task_type;
  desc.add_options()
    ("help,h", "Show help")
    ("type,t", po::value<std::string>(&task_type), "Select task: train, recognize, score")
    ;
  po::options_description train_desc("Train options");
  train_desc.add_options()
    ("input,I", po::value<std::string>(), "Input .dat file")
    ("info,i", po::value<std::string>(), "Input .trn file")
    ("output,O", po::value<std::string>(), "Output parameters file .prs")
    ;
  po::options_description recognize_desc("Recognize options");
  recognize_desc.add_options()
    ("input,I",  po::value<std::vector<std::string> >(), "Input .dat file")
    ("params,p", po::value<std::string>(), "Input .prs file")
    ("output,O", po::value<std::string>(), "Output directory")
    ;
  po::options_description score_desc("Score options");
  score_desc.add_options()
    ("ethanol,e",  po::value<std::string>(), "Etalon .trn file")
    ("test,t", po::value<std::string>(), "Testing .trn file")
    ("output,O", po::value<std::string>(), "Output comparison file")
    ;
  po::variables_map vm;
  try {
    po::parsed_options parsed = po::command_line_parser(ac, av).options(desc).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);
    if(task_type == "train") {
      desc.add(train_desc);
      po::store(po::parse_command_line(ac,av,desc), vm);
      train(vm);
    }
    else if(task_type == "recognize") {
      desc.add(recognize_desc);
      po::store(po::parse_command_line(ac,av,desc), vm);
      recognize(vm);
    }
    else if(task_type=="score") {
      desc.add(score_desc);
      po::store(po::parse_command_line(ac,av,desc), vm);
      score(vm);
    }
    else {
      desc.add(train_desc).add(recognize_desc).add(score_desc);
      std::cout << desc << std::endl;
    }
  }
  catch(std::exception& ex) {
    std::cout << desc << std::endl;
  }	
	return 0;
}
//-------------------------------------------------------------------------
