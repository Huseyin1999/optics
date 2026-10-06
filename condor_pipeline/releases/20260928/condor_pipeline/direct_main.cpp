// File-oriented entry point for the HTCondor pipeline.
// The legacy implementation is included under a different main symbol so all
// optical processing code stays shared without modifying TAIGA_optics itself.
#define main taiga_optics_legacy_main
#include "../TAIGA_optics/main.cpp"
#undef main

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace
{
void PrintFileUsage()
{
  std::cerr << "Usage: TAIGA_optics_file --input-file FILE --output-prefix PREFIX "
            << "[--parameters FILE]" << std::endl;
}

bool ReadRunSettings(const std::string &path, std::vector<Telescope_parameters> &parameters,
                     double &da, double &theta, double &phi, int &hs_trigger)
{
  FILE *config=std::fopen(path.c_str(), "r");
  if(config==nullptr)
  {
    std::cerr << "Could not open config file: " << path << std::endl;
    return false;
  }

  char filename[2000];
  char line[800];
  int unused_start, unused_runs, source_model, wobble;
  double wobble_r, wobble_phi, wobble_rate, wobble_dt, wobble_rotation;

  // The first three path templates belong to legacy run-number mode. They are
  // parsed for format compatibility but deliberately ignored here.
  if(std::fscanf(config, "%1999s", filename)!=1 ||
     std::fscanf(config, "%1999s", filename)!=1 ||
     std::fscanf(config, "%1999s\n", filename)!=1 ||
     std::fgets(line, sizeof(line), config)==nullptr ||
     std::sscanf(line, "%d %d %lf %d", &unused_start, &unused_runs, &da, &source_model)!=4 ||
     std::fgets(line, sizeof(line), config)==nullptr ||
     std::sscanf(line, "%lf %lf %d", &theta, &phi, &hs_trigger)!=3 ||
     std::fscanf(config, "%1999s\n", filename)!=1 ||
     std::fgets(line, sizeof(line), config)==nullptr ||
     std::sscanf(line, "%d %lf %lf %lf %lf %lf", &wobble, &wobble_r, &wobble_phi,
                 &wobble_rate, &wobble_dt, &wobble_rotation)!=6)
  {
    std::cerr << "Invalid config format in: " << path << std::endl;
    std::fclose(config);
    return false;
  }
  std::fclose(config);

  theta*=std::acos(-1.0)/180.0;
  phi*=std::acos(-1.0)/180.0;
  parameters[0].path_atm=filename;
  parameters[0].source_model=source_model;
  parameters[0].wobble=wobble;
  parameters[0].wobble_r=wobble_r;
  parameters[0].wobble_phi=wobble_phi;
  parameters[0].wobble_rate=wobble_rate;
  parameters[0].wobble_dt=wobble_dt;
  parameters[0].wobble_frame_rotation_speed=wobble_rotation;
  return true;
}
}

int main(int argc, char **argv)
{
  std::string input;
  std::string output_prefix;
  std::string parameters_path="parameters.txt";

  for(int i=1;i<argc;i++)
  {
    const std::string option=argv[i];
    if(option=="--input-file" && i+1<argc)
      input=argv[++i];
    else if(option=="--output-prefix" && i+1<argc)
      output_prefix=argv[++i];
    else if(option=="--parameters" && i+1<argc)
      parameters_path=argv[++i];
    else
    {
      PrintFileUsage();
      return 2;
    }
  }
  if(input.empty() || output_prefix.empty())
  {
    PrintFileUsage();
    return 2;
  }

  std::vector<Telescope_parameters> parameters(1);
  parameters[0].Load(parameters_path);
  if(parameters[0].path_config.empty())
  {
    std::cerr << "Parameters do not define path_config: " << parameters_path << std::endl;
    return 1;
  }

  double da, theta, phi;
  int hs_trigger;
  if(!ReadRunSettings(parameters[0].path_config, parameters, da, theta, phi, hs_trigger))
    return 1;

  int parameter_number=1;
  std::string extra_path;
  FILE *extra=nullptr;
  const std::string::size_type extension=parameters_path.rfind(".txt");
  if(extension!=std::string::npos)
  {
    while((extra=std::fopen((extra_path=parameters_path.substr(0, extension)+
                            Form("_t%d", ++parameter_number)+".txt").c_str(), "rt"))!=nullptr)
    {
      std::fclose(extra);
      parameters.resize(parameter_number);
      parameters.back().Load(extra_path);
      parameters.back().path_atm=parameters[0].path_atm;
      parameters.back().source_model=parameters[0].source_model;
    }
  }

  process_binary_files(input, output_prefix, parameters, 0, 1, da, 1,
                       theta, phi, hs_trigger);
  return 0;
}
