#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <utility>
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>
#include <map>
#include <omp.h>
#include <math.h>
#include "Math/Interpolator.h"
using namespace std;
FILE *f5;
string line;
ROOT::Math::Interpolator inter(ROOT::Math::Interpolation::kAKIMA_PERIODIC);
typedef pair <int, int> type_key;
map <type_key, int> Mapa;
//double fy = 0, fb = 1, fa = 6., fx = 0, fc = 2.03, fd = 2.5;
//double fymx = 0;
//map <pair, int> pmt_coord;
//vector<Photoelectron> pe_focal;
//map<int32_t, int32_t> pixN;
//vector<int32_t> a_hs;
//vector<double> t_hs;
//vector<double> dt_hs;

int32_t a_final;
int mirrow, mircol, bad_mirror, miss_phe, evch = 0;

void readSlowPulse(string path, int len_pulse, double t_min, double amp_max){
	cout << path << endl;
	ifstream file0(path);
	if (!file0.is_open()) {
		cout << "Файл slow_pulse не найден" << endl;
	}
	else {
		double pulse[len_pulse][2];
		vector<double> vector_x;
		vector<double> vector_y;
		for(int ii = 0; ii < len_pulse; ii++) {
			getline(file0, line);
			stringstream ist(line);
			for(int i = 0; i < 2; i++) {
				ist >> pulse[ii][i];
				cout << pulse[ii][i] << "\t";
			}
			vector_x.push_back(pulse[ii][0]-t_min);
			vector_y.push_back(pulse[ii][1]/amp_max);
			cout << endl;
		}
		inter.SetData(vector_x, vector_y);
	}
	file0.close();
}

int mod(int n, int d)
{
	int result = n % d;
	if ((result * d) < 0)
		result += d;
	return result;
}
/*
   void readRowColClust(){
        ifstream file0("RowColClust560_fix_375.txt");
        if (!file0.is_open()) {
                cout << "Файл не найден" << endl;
        }
        else {
                for(int ii = 0; ii < number_of_pixels; ii++) {
                        getline(file0, line);
                        stringstream ist(line);
                        for(int i = 0; i < 5; i++) {
                                ist >> rowcol[ii][i];
                                //cout << rowcol[ii][i] << "\t";
                        }
                        rowcol[ii][2] = rowcol[ii][2] - 1;
                        cout << ii << "\t" << rowcol[ii][2] << endl;
                        key[ii] = make_pair(rowcol[ii][0], rowcol[ii][1]);
                        Mapa[key[ii]] = ii;
                }
        }
        file0.close();
   }

   void readNeighbours(){
        ifstream file0("neighbour_cluser_pairs_iact1");
        if (!file0.is_open()) {
                cout << "Файл не найден" << endl;
        }
        else {
                for(int ii = 0; ii < number_of_pixels; ii++) {
                        getline(file0, line);
                        stringstream ist(line);
                        for(int i = 0; i < 9; i++) {
                                ist >> neigh[ii][i];
                                /*if(neigh[ii][i] == -1){
                                   neigh[ii][i] = ii;
                                   }*/
/*			}
                }
        }
        file0.close();
   }

   void readNeighbours_clean(){
        ifstream file1("neighbour_clean_corsica_iact1_375.txt");
        if (!file1.is_open()) {
                cout << "Файл не найден" << endl;
        }
        else {
                for(int ii = 0; ii < number_of_pixels; ii++) {
                        getline(file1, line);
                        stringstream ist(line);
                        for(int i = 0; i < 13; i++) {
                                ist >> neigh_clean[ii][i];
                        }
                }
        }
        file1.close();
   }

   void readChNumbs_clean(){
        ifstream file4("neighbour_clean_experiment_iact1_375.txt");
        if (!file4.is_open()) {
                cout << "neighbour_clean Файл не найден" << endl;
        }
        else {
                for(int ii = 0; ii < number_of_pixels; ii++) {
                        getline(file4, line);
                        stringstream ist(line);
                        for(int i = 0; i < 2; i++) {
                                ist >> real_numb[ii][i];
                        }
                }
        }
        file4.close();
   }
 */
/*void read_cam_geom(){
        ifstream file("IACT01_cam_corsica_config.csv");
        if (!file.is_open()) {
                cout << "IACT01_cam_corsica_config Файл не найден" << endl;
        }
        else {
                string source;
                getline(file, line);
                for(int ii = 0; ii < number_of_pixels; ii++) {
                        getline(file, line);
                        stringstream ist(line);
                        for(int i = 0; i < 15; i++) {
                                getline(ist, source, ',');
                                if(i==0) {//pix_numb
                                        neigh[ii][0]=atoi(source.c_str());
                                }
                                if(i==1) {//cluster
                                        rowcol[ii][2]=atoi(source.c_str())-1;
                                        //cout << rowcol[ii][2] << endl;
                                        real_numb[ii][0]=atoi(source.c_str());
                                }
                                if(i==2) {//channel
                                        real_numb[ii][1]=atoi(source.c_str());
                                }
                                if(i==3) {//row
                                        rowcol[ii][0]=atoi(source.c_str());
                                }
                                if(i==4) {//col
                                        rowcol[ii][1]=atoi(source.c_str());
                                }
                                if(i==5) {//x_cam
                                        coord[ii][0]=atof(source.c_str());
                                }
                                if(i==6) {//x_cam
                                        coord[ii][1]=atof(source.c_str());
                                }
                                if(i==7) {//trig
                                        neigh[ii][1]=atoi(source.c_str());
                                }
                                if(i==8) {//n_neighbours
                                        neigh[ii][2]=atoi(source.c_str());
                                }
                                if(i>=9) {//neighbour_index
                                        neigh[ii][3+i-9]=atoi(source.c_str());
                                }
                        }
                        key[ii] = make_pair(rowcol[ii][0], rowcol[ii][1]);
                        Mapa[key[ii]] = ii;
                }
        }
   }
 */
vector<double> random_poisson(double time_begin, double time_end, double mean_ph, double mean_ph_time)
{
	//double mean_ph = 3; //mean number of NSB photons
	//double mean_ph_time = 35; //period with 2 NSB photons
	vector<double> result;
	// construct a trivial random generator engine from a time-based seed:
	long int seed = std::chrono::system_clock::now().time_since_epoch().count();
	default_random_engine generator (seed);
	poisson_distribution<int> distribution (mean_ph*(time_end-(time_begin-1300))/mean_ph_time);
	double numb_NSB_phe = distribution(generator);
	random_device myRandomDevice;
	unsigned seed2 = myRandomDevice();
	default_random_engine generator2 (seed2);
	uniform_real_distribution<double> distribution2(time_begin-1300, time_end);
	for(int t = 0; t <= round(numb_NSB_phe); t++) {
		result.push_back(distribution2(generator2));
	}
	return result;
}

double amp_t(double t, double ampmx){
	double y = 0, b = 1, a = 6., c = 2.03, d = 2.5;
	double x = t - a;
	double ymx = b * exp(-pow((c/(2*d)),2))*exp(pow(c,2)/(2*pow(d,2)));
	if(t < a) {
		y=b*exp(-pow((x/c),2))*exp(-x/d)*ampmx/ymx;
	}
	else{
		y = b*exp(-x/d)*ampmx/ymx;
	}
	return y;
}

vector<float> amp_distr(string path)
{
	vector<float> result;
	ifstream file(path);
	if (!file.is_open()) {
		cout << "Файл распределения амлитуд не найден" << endl;
		exit(0);
	}
	else {
		getline(file, line);
		stringstream ist(line);
		while(ist) {
			float amp = 0;
			ist >> amp;
			if(amp > 0.6) {
				result.push_back(amp);
			}
		}
		file.close();
	}
	return result;
}

int main(int argc, char **argv)
{
	int selected_tel = atoi(argv[1]);
	int number_of_pixels = atoi(argv[2]);
	int numb_of_clusters = atof(argv[3]); //потому что массив от нуля а у нас в файле запись от 1
	int trigger_type = atoi(argv[4]);
	double t_grid = atof(argv[5]); //время между записями во временной сетке
	double t_sign = atof(argv[6]); //время в течении которого считаем что сигнал от фотоэлектрона значим
	double trig_amp = atof(argv[7]); //превышение данной амплитуды дает холд в подтягиваемом кластере и окно 80 нс и 15 нс для холдов и второго пикселя в этом кластере соответственно
	double t_hold = atof(argv[8]); //длинна сигнала HOLD
	double integrate_window = atof(argv[9]); //время, в течении которого суммируем фотоэлектроны
	double trig_window = atof(argv[10]); //окно триггера
	double t_after_before = atof(argv[11]); //отступ от первого черенковского фэ и после последнего черенковского фэ
	int pix1, pix2;
	int len_pulse = atoi(argv[12]);
	double interpol1 = atof(argv[13]);
	double interpol2 = atof(argv[14]);
	int sim_back = atoi(argv[15]);
	double poissin_time = atof(argv[16]);
	double poissin_mean = atof(argv[17]);
	double inf = numeric_limits<double>::infinity();
	int32_t N_run, i_scat, i_tel, N_b_f, N_shower, N_phe, read32[4], history, roco[2];
	double buf_header_angles[5], phe[2], tim_min, tim_max, amp;
	double print_buf_head[15], readdubble[20], rowcol[number_of_pixels][3],sense[number_of_pixels], integral35, coord[number_of_pixels][2], hold, trig_time, amp1, amp2;

	int neigh[number_of_pixels][10], trigger, trig_clust, trig_hold, grid_max, cluster, real_numb[number_of_pixels][2];
	int list_numb = 0;
	int list_scat = 0;
	double coef_sens = atof(argv[23]);
	int hidmir[atoi(argv[24])][2];
	if(atoi(argv[24])>0) {
		for(int i=0; i<atoi(argv[24]); i++) {
			hidmir[i][0]=atoi(argv[25+2*i]);
			hidmir[i][1]=atoi(argv[25+2*i+1]);
		}
	}
	type_key key[number_of_pixels];
	double tet_center, fi_center, tet_source, fi_source;
	char notification[150];

	ifstream file(argv[18]);
	cout << argv[18] << endl;
	if (!file.is_open()) {
		sprintf(notification, "%s%s%s", "IACT", argv[1], "_cam_corsica_config Файл не найден");
		cout << notification << endl;
	}
	else {
		string source;
		getline(file, line);
		for(int ii = 0; ii < number_of_pixels; ii++) {
			getline(file, line);
			stringstream ist(line);
			for(int i = 0; i < 15; i++) {
				getline(ist, source, ',');
				if(i==0) {//pix_numb
					neigh[ii][0]=atoi(source.c_str());
				}
				if(i==1) {//cluster
					rowcol[ii][2]=atoi(source.c_str())-1;
					//cout << rowcol[ii][2] << endl;
					real_numb[ii][0]=atoi(source.c_str());
				}
				if(i==2) {//channel
					real_numb[ii][1]=atoi(source.c_str());
				}
				if(i==3) {//row
					rowcol[ii][0]=atoi(source.c_str());
				}
				if(i==4) {//col
					rowcol[ii][1]=atoi(source.c_str());
				}
				if(i==5) {//x_cam
					coord[ii][0]=atof(source.c_str());
				}
				if(i==6) {//x_cam
					coord[ii][1]=atof(source.c_str());
				}
				if(i==7) {//trig
					neigh[ii][1]=atoi(source.c_str());
				}
				if(i==8) {//sense
					sense[ii]=atof(source.c_str());
				}
				if(i==9) {//n_neighbours
					neigh[ii][2]=atoi(source.c_str());
				}
				if(i>=10) {//neighbour_index
					neigh[ii][3+i-10]=atoi(source.c_str());
				}
			}
			key[ii] = make_pair(rowcol[ii][0], rowcol[ii][1]);
			Mapa[key[ii]] = ii;
		}
		for(int ii = 0; ii < number_of_pixels; ii++) {
			for(int i = 0; i < neigh[ii][2]; i++){
				if(real_numb[ii][0] != real_numb[neigh[ii][3+i]][0] || neigh[ii][1] != 1 || neigh[neigh[ii][3+i]][1] != 1){
					neigh[ii][3+i] = -1;
				}
			}
		}
	}
	//inter(atoi(argv[12]), ROOT::Math::Interpolation::kAKIMA_PERIODIC)
	readSlowPulse(argv[22], len_pulse, interpol1, interpol2);
	/*readRowColClust();
	   readNeighbours_clean();
	   readNeighbours();
	   readChNumbs_clean();*/
	//read_cam_geom();
	cout << "ok" << endl;
	//read32: N_shower, i_scat, i_tel, N_phe
	//readdubble: energy, zen, az, x_core, y_core, z_core, height_1_interaction, particle_type,
	//xmax, hmax, xtel, ytel, ztel, Xoffsetmm, Yoffsetmm, Thetatelrad, Phitelrad, alfarad, alfamaxrad,
	//T
	vector <float> vector_amp_phe;
	cout << "getting vector of phe amps" << endl;
	vector_amp_phe = amp_distr(argv[19]);
	cout << "vector complitely obtaned" << endl;
	ofstream fout(argv[20]);
	//ofstream fout100("/hdd/IACTs/Corsika/readbin/data/new_cone/1km/Crab/bpe607_31_da1.2_md5_31v_0deg/source_coords.txt");
	cout << argv[21] << endl;
	f5 = fopen(argv[21], "rf");
	int event = 0;
	while(1) {
		//cout << event << endl;
		miss_phe = 0;
		tim_min = inf;
		tim_max = -inf;
		fread(&read32, sizeof(int32_t), 4, f5);
		if(feof(f5) != 0) {
			cout << "Файл закончился или пустой" << endl;
			break;
		}
		if(read32[3] < 0) {
			cout << "Wrong number photoelectrons: " << read32[3] << endl;
			break;
		}
		//cout << read32[0] << "\t" << read32[1] << endl;
		if(read32[2] == (int)(selected_tel-1)) {
			vector<vector<double> > vector_phe_time(number_of_pixels);
			vector<vector<double> > vector_amp(number_of_pixels);
			vector<vector<double> > vector_amp_MC(number_of_pixels);
			vector<vector<double> > vector_integral35(number_of_pixels);
			vector<vector<double> > vector_integral_hold(numb_of_clusters);
			vector<vector<double> > vector_time10(number_of_pixels);
			vector<vector<double> > vector_prelim_hold_time(numb_of_clusters);
			vector<vector<int> > vector_prelim_hold_pix(numb_of_clusters);
			fread(&readdubble, sizeof(double), 20, f5);
			tet_center = readdubble[15];
			fi_center =  readdubble[16];
			tet_source = readdubble[1];
			fi_source =  readdubble[2];

			//fi_source = M_PI - fi_source;
			//fi_center = M_PI - fi_center;
			//double XToSourcecamatan2_cm = ((475./(sin(tet_source)*sin(tet_center) + cos(tet_source)*cos(tet_center)*cos(fi_source-fi_center)))*(cos(tet_source)*sin(fi_source-fi_center)));
			//double YToSourcecamatan2_cm = ((475./(sin(tet_source)*sin(tet_center) + cos(tet_source)*cos(tet_center)*cos(fi_source-fi_center)))*(cos(tet_source)*sin(tet_center)*cos(fi_source - fi_center) - sin(tet_source)*cos(tet_center)));
			//cout << readdubble[1] << "\t" << readdubble[15] << "\t" << readdubble[2] << "\t" << readdubble[16] << "\t" << readdubble[17] << "\t" << XToSourcecamatan2_cm << "\t" << YToSourcecamatan2_cm << endl;
			//fout100 << XToSourcecamatan2_cm << "\t" << YToSourcecamatan2_cm << endl;

			tim_max = -inf;
			tim_min = inf;
			for(int phe_i = 0; phe_i < read32[3]; phe_i++) {
				bad_mirror = 0;
				phe[0] = 0;
				fread(&history, sizeof(int32_t), 1, f5);
				fread(&phe, sizeof(double), 2, f5);         //time, ns and wavelength, nm
				if(phe[1] < 100 || phe[1] > 800) {
					cout << "Wrong wl: " << phe[1] << endl;
					break;
				}
				if(phe[0]*1E9 > tim_max) { tim_max = phe[0]*1E9;}
				if(phe[0]*1E9 < tim_min) { tim_min = phe[0]*1E9;}
				fread(&roco, sizeof(int32_t), 2, f5);
				mirrow = (int)floor((double)history / 256);
				mircol = mod(history,256) - 128;
				if(atoi(argv[24])>0) {
					for(int i = 0; i < atoi(argv[24]); i++) {
						if(hidmir[i][0] == mirrow && hidmir[i][1] == mircol) {
							//cout << "excluded\t" << phe_i << "\t" << history << "\t" << floor((double)history / 256) << "\t" << mod(history,256) - 128 << endl;
							bad_mirror = 1;
							miss_phe++;
							break;
						}
					}
				}
				
				if(bad_mirror == 0) {
					//cout << "\t\t" << phe_i << "\t" << history << "\t" << floor((double)history / 256) << "\t" << mod(history,256) - 128 << endl;
					
					map <type_key, int> :: iterator it;
					it = Mapa.find({roco[0], roco[1]});
					if (it->second != 0 || (roco[0] == -3 && roco[1] != 0)) {
    					vector_phe_time[it->second].push_back(phe[0] * 1E9);
					}
				}
				// else{
				// 	cout << mirrow << "\t" << mircol << endl;
				// }
			}
			//cout << read32[0] << "\t" << read32[1] << endl;
			//cout << tim_max - tim_min << "\t" << tim_min << "\t" << tim_max << endl;
			//cout << vector_phe_time.size() << endl;;
			if(tim_max - tim_min > 1000) {
				tim_max = 0;
				tim_min = 0;
			}
			/*for(int i = 0; i < (int)vector_phe_time.size(); i++) {
			        cout << i << "\t" << vector_phe_time[i].size() << endl;
			   }*/
			if(sim_back == 2) {
				tim_min = tim_min - t_after_before;
				tim_max = tim_max + t_after_before;
				for(int i = 0; i < (int)vector_phe_time.size(); i++) {
					if(vector_phe_time[i].size() != 0 ) {
						vector <double> timesNSBphe = random_poisson(tim_min,tim_max, poissin_mean, poissin_time);
						vector_phe_time[i].insert(vector_phe_time[i].end(), timesNSBphe.begin(), timesNSBphe.end());
						sort(vector_phe_time[i].begin(), vector_phe_time[i].end());
					}
					else{
						vector_phe_time[i] = random_poisson(tim_min,tim_max, poissin_mean, poissin_time);
						sort(vector_phe_time[i].begin(), vector_phe_time[i].end());
					}
					for(int t = 0; t < (int)vector_phe_time[i].size(); t++) {
						vector_amp_MC[i].push_back(vector_amp_phe[rand()%vector_amp_phe.size()]);
						//vector_amp_MC[i].push_back(1.);
						
					}
				}
			}
			else{
				for(int i = 0; i < (int)vector_phe_time.size(); i++) {
					for(int t = 0; t < (int)vector_phe_time[i].size(); t++) {
						//cout << t << "\t" << i << "\t" << real_numb[i][0] << "\t" << real_numb[i][1] << "\t" << sense[i] << endl;
						//vector_amp_MC[i].push_back(1);
						//double rand_value = ((double) rand() / (RAND_MAX));
						if(((double) rand() / (RAND_MAX)) < sense[i]*coef_sens){
							vector_amp_MC[i].push_back(vector_amp_phe[rand()%vector_amp_phe.size()]);
						}
						else{
							vector_amp_MC[i].push_back(0);
						}
					}
					//if(read32[0] >= 533){
					//cout << read32[0] << "\t" << read32[1] << "\t" << i << "\t" << vector_amp_MC[i].size() << endl;
						//exit(0);
					//}
				}
			}
			trigger = 0;
			trig_time = -inf;
			trig_clust = -1;
			pix1 = -1;
			pix2 = -1;
			amp1 = -1;
			amp2 = -1;
			//omp_set_num_threads(560);
			int i, ii, j;
			double t;
			vector <double> vector_time_grid;
			//cout << tim_min << "\t" << tim_max << endl;
			for(t = tim_min; t <= tim_max; t=t+t_grid) {                 //идем по сетке от минимального в событии времени до максимального
				vector_time_grid.push_back(t);
			}
			//cout << number_of_pixels << "\t" << vector_time_grid.size() << endl;
			//double amp_array[number_of_pixels][(int)vector_time_grid.size()];
			vector<std::vector<double> > amp_array(number_of_pixels,std::vector<double>((int)vector_time_grid.size()));
			/*cout << read32[0] << "\t" << read32[1] << "\t" << number_of_pixels << "\t" << vector_time_grid.size() << endl;
			double sum_zeros = 0;
			for (int count = 0; count < number_of_pixels; count++)
			{
				for (int coun = 0; coun < (int)vector_time_grid.size(); coun++)
				{
					sum_zeros = sum_zeros + amp_array[count][coun];
				}
			}
			cout << "sum " <<  sum_zeros << endl;*/
//			int j = 0;
//#pragma omp parallel shared(rowcol, vector_time10, vector_prelim_hold_time, vector_prelim_hold_pix, vector_phe_time, vector_amp_MC, t_sign, tim_min, tim_max, trig_amp, amp_array, vector_time_grid) private(j, ii, i)
//			{
// #pragma omp for
			//fout1 << event << "\t" << (tim_max - tim_min) << endl;
			for(i = 0; i < (int)vector_phe_time.size(); i++) {         //идем по сработавшим пикселям
				if(vector_phe_time[i].size() != 0 && neigh[i][1] == 1) {         //если он не пустой и если он триггерный:
					for(ii = 0; ii < (int)vector_phe_time[i].size(); ii++) {         //идем по конкретным значениям времён прихода фотоэлектронов
						for(j = 0; j < (int)vector_time_grid.size(); j++) {         //идем по сетке от минимального в событии времени до максимального
							if((vector_time_grid[j] >= vector_phe_time[i][ii]) && (vector_time_grid[j] <= (vector_phe_time[i][ii] + t_sign))) {         //если время сетки идет после прихода фотоэлектрона(не посзже 30 ns):
								amp_array[i][j] = amp_array[i][j] + amp_t((vector_time_grid[j] - vector_phe_time[i][ii]), vector_amp_MC[i][ii]);
							}
						}
					}
				}
			}
//			}
			if(trigger_type == 0) {
				for(i = 0; i < (int)vector_phe_time.size(); i++) {
					for(j = 0; j < (int)vector_time_grid.size(); j++) {
						if(amp_array[i][j] > trig_amp) {
							vector_time10[i].push_back(vector_time_grid[j]);
							vector_prelim_hold_time[(int)rowcol[i][2]].push_back(vector_time_grid[j]);
							vector_prelim_hold_pix[(int)rowcol[i][2]].push_back(i);
						}
					}
				}
				for(int i = 0; i < numb_of_clusters; i++) {
					sort(vector_prelim_hold_time[i].begin(), vector_prelim_hold_time[i].end());
					hold = -inf;
					for(int ii = 0; ii < vector_prelim_hold_time[i].size(); ii++) {
						if(vector_prelim_hold_time[i][ii] > (hold + t_hold)) {
							hold = vector_prelim_hold_time[i][ii];
							vector_integral_hold[i].push_back(hold);
						}
					}
				}
				for(int i = 0; i < (int)vector_phe_time.size(); i++) {
					if(vector_time10[i].size() != 0) {
						//cout << event << "\t" << i << "\t" << vector_time10[i].size() << endl;
						if(vector_integral_hold[(int)rowcol[i][2]].size() > 0) {
							for(int ii = 0; ii < neigh[i][2]; ii++) {
								//cout << i << "\tneigh\t" << neigh[i][3+ii]  << endl;
								if(neigh[i][3+ii] != -1){
									//if(vector_integral_hold[(int)rowcol[neigh[i][3+ii]][2]].size() > 0){
									if(vector_time10[neigh[i][3+ii]].size() != 0) {
										for(int j = 0; j < vector_time10[i].size(); j++) {
											for(int jj = 0; jj < vector_time10[neigh[i][3+ii]].size(); jj++) {
												//cout << event << "\t" << i << "\t" << neigh[i][3+ii] << "\t" << vector_time10[i][j] << "\t" << vector_time10[neigh[i][3+ii]][jj] << endl;
												if(((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) <= trig_window) && ((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) >= 0)) {
													for(int ij = 0; ij < vector_integral_hold[(int)rowcol[i][2]].size(); ij++) {
														//cout << vector_time10[i][j] << "\t" << vector_integral_hold[(int)rowcol[i][2]][ij] << endl;
														if(abs(vector_time10[neigh[i][3+ii]][jj] - vector_integral_hold[(int)rowcol[neigh[i][3+ii]][2]][ij]) < 0.1) {
															//cout << "trig" << endl;
															trigger = 1;
															trig_time = vector_time10[neigh[i][3+ii]][jj];
															trig_clust = rowcol[neigh[i][3+ii]][2];
															pix2 = i;
															pix1 = neigh[i][3+ii];
															amp2 = vector_time10[i][j];
															amp1 = vector_time10[neigh[i][3+ii]][jj];
															goto exit1;
														}
													}
												}
												else if(((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) >= -trig_window) && ((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) <= 0)) {
													//cout << vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj] << endl;
													for(int ij = 0; ij < vector_integral_hold[(int)rowcol[neigh[i][3+ii]][2]].size(); ij++) {
														//cout << vector_time10[neigh[i][3+ii]][jj] << "\t" << vector_integral_hold[(int)rowcol[neigh[i][3+ii]][2]][ij] << endl;
														if(abs(vector_time10[i][j] - vector_integral_hold[(int)rowcol[i][2]][ij]) < 0.1) {
															//cout << "trig" << endl;
															trigger = 1;
															trig_time = vector_time10[i][j];
															trig_clust = rowcol[i][2];
															pix1 = i;
															pix2 = neigh[i][3+ii];
															amp1 = vector_time10[i][j];
															amp2 = vector_time10[neigh[i][3+ii]][jj];
															goto exit1;
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
exit1:                          ;
				if(trigger == 1) {
					//cout << event  << "\t" << "trig" << endl;
					vector <double> vector_event_amp;
					vector <int> vector_event_numb;
					vector <double> vector_event_amp1;
					vector <int> vector_event_numb1;
					for(int i = 0; i < (int)vector_phe_time.size(); i++) {
						if(vector_phe_time[i].size() != 0) {
							//cout << (int)rowcol[i][2] << endl;
							if(vector_integral_hold[(int)rowcol[i][2]].size() > 0) {
								for(int ii = 0; ii < vector_integral_hold[(int)rowcol[i][2]].size(); ii++) {
									if(vector_integral_hold[(int)rowcol[i][2]][ii] >= trig_time - t_hold/2 && vector_integral_hold[(int)rowcol[i][2]][ii] <= trig_time + t_hold/2) { //т.к. через 80нс после триггера происходит считывание тех кластеров, у которых на этот момент есть холд
										amp = 0;
										amp1 = 0;
										for(int j = 0; j < (int)vector_phe_time[i].size(); j++) { //идем по конкретным значениям времён прихода фотоэлектронов
											if(vector_phe_time[i][j] > vector_integral_hold[(int)rowcol[i][2]][ii] - 600 && vector_phe_time[i][j] <= vector_integral_hold[(int)rowcol[i][2]][ii] + integrate_window) {
												amp = amp + vector_amp_MC[i][j]*inter.Eval(vector_integral_hold[(int)rowcol[i][2]][ii] + integrate_window - vector_phe_time[i][j]);
												//cout << i << "\t" << vector_phe_time[i][j] << "\t" << vector_integral_hold[(int)rowcol[i][2]][ii]+35 << "\t" <<  vector_integral_hold[(int)rowcol[i][2]][ii]+35 - vector_phe_time[i][j] << "\tamp:\t" << inter.Eval(vector_integral_hold[(int)rowcol[i][2]][ii] + 35 - vector_phe_time[i][j]) << "\t" << amp << endl;
											}
											if(vector_phe_time[i][j] >= vector_integral_hold[(int)rowcol[i][2]][ii] && vector_phe_time[i][j] <= (vector_integral_hold[(int)rowcol[i][2]][ii] + integrate_window)) {
												amp1 = amp1 + vector_amp_MC[i][j];
											}
										}
										vector_event_amp.push_back(amp);
										vector_event_numb.push_back(i);

										vector_event_amp1.push_back(amp1);
										vector_event_numb1.push_back(i);
										//cout << read32[0] << "\t" << read32[1] << "\t" << i << endl;
										break;
									}
								}
							}
						}
						else if(vector_integral_hold[(int)rowcol[i][2]].size() > 0) {
							vector_event_amp.push_back(0);
							vector_event_numb.push_back(i);
						}
					}
					if(vector_event_numb.size() > 0) {
						cout << evch << "\t" << read32[2] << "\t" << read32[0] << "\t" << read32[1] << "\t" << vector_event_amp.size() << "\t" << readdubble[0]/1E12 << "\t" << (readdubble[10] + readdubble[13])/1E3 << "\t " << (readdubble[11] + readdubble[14])/1E3 << endl;
						//fout << read32[0] << "\t" << read32[1] << "\t" << vector_event_amp.size() << "\t" << readdubble[0]/1E12 << "\t" << XToSourcecamatan2_cm << "\t" << YToSourcecamatan2_cm << "\t" << (readdubble[10] + readdubble[13])/1E3 << "\t" << (readdubble[11] + readdubble[14])/1E3 << endl;
						fout << read32[0] << "\t" << read32[1] << "\t" << vector_event_amp.size() << "\t" << readdubble[0]/1E12 << "\t" << (readdubble[10] + readdubble[13])/1E3 << "\t" << (readdubble[11] + readdubble[14])/1E3 << "\t" << readdubble[15] << "\t" << readdubble[16] << "\t" << readdubble[1] << "\t" << readdubble[2] << "\t" << readdubble[8] << endl;
						//fout1 << read32[0] << "\t" << read32[1] << "\t" << trig_clust+1  << "\t" << vector_event_amp.size() << "\t" << readdubble[0]/1E12 << "\t" << R << "\t"<< readdubble[17]*180/Pi << endl;
						for(int i = 0; i < (int)vector_event_numb.size(); i++) {
							//cout << neigh_clean[vector_event_numb[i]][1] << "\t" << neigh_clean[vector_event_numb[i]][2] << "\t" << neigh_clean[vector_event_numb[i]][3] << "\t"<< neigh_clean[vector_event_numb[i]][4] << "\t" << vector_event_amp[i] << endl;
							fout << real_numb[vector_event_numb[i]][0] << "\t" << real_numb[vector_event_numb[i]][1] << "\t" << coord[vector_event_numb[i]][0] << "\t"<< coord[vector_event_numb[i]][1] << "\t" << vector_event_amp[i] << endl;
							//fout1 << real_numb[vector_event_numb1[i]][1] << "\t" << real_numb[vector_event_numb1[i]][2] << "\t" << neigh_clean[vector_event_numb1[i]][3] << "\t"<< neigh_clean[vector_event_numb1[i]][4] << "\t" << vector_event_amp1[i] << endl;
						}
					}
					for(int i = 0; i < (int)vector_event_numb.size(); i++) {
						vector_event_amp.clear();
						vector_event_numb.clear();
						vector_event_amp1.clear();
						vector_event_numb1.clear();
					}
					evch++;
				}
			}
			else if (trigger_type == 1) {
				for(i = 0; i < (int)vector_phe_time.size(); i++) {
					for(j = 0; j < (int)vector_time_grid.size(); j++) {
						if(amp_array[i][j] > trig_amp) {
							vector_time10[i].push_back(vector_time_grid[j]);
						}
					}
				}
				for(int i = 0; i < (int)vector_phe_time.size(); i++) {
					if(vector_time10[i].size() != 0) {
						//cout << event << "\t" << i << "\t" << vector_time10[i].size() << endl;
						for(int ii = 0; ii < neigh[i][2]; ii++) {
							//cout << i << "\tneigh\t" << neigh[i][3+ii]  << endl;
							if(neigh[i][3+ii] != -1){
								//if(vector_integral_hold[(int)rowcol[neigh[i][3+ii]][2]].size() > 0){
								if(vector_time10[neigh[i][3+ii]].size() != 0) {
									for(int j = 0; j < vector_time10[i].size(); j++) {
										for(int jj = 0; jj < vector_time10[neigh[i][3+ii]].size(); jj++) {
											//cout << event << "\t" << i << "\t" << neigh[i][3+ii] << "\t" << vector_time10[i][j] << "\t" << vector_time10[neigh[i][3+ii]][jj] << endl;
											if(((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) <= trig_window) && ((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) >= 0)) {
												//cout << "trig" << endl;
												trigger = 1;
												trig_time = vector_time10[neigh[i][3+ii]][jj];//вероятно время триггера - это время первого превысевшего порог пикселя, так что тут должно быть время соседа
												trig_clust = rowcol[neigh[i][3+ii]][2];
												pix2 = i;//вероятно везеде нужно поменять местами значения 1 и 2, ведь первым был сосед
												pix1 = neigh[i][3+ii];
												amp2 = vector_time10[i][j];
												amp1 = vector_time10[neigh[i][3+ii]][jj];
												goto exit2;
											}
											else if(((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) >= -trig_window) && ((vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj]) <= 0)) {
												//cout << vector_time10[i][j] - vector_time10[neigh[i][3+ii]][jj] << endl;
												//cout << "trig" << endl;
												trigger = 1;
												trig_time = vector_time10[i][j];
												trig_clust = rowcol[i][2];
												pix1 = i;
												pix2 = neigh[i][3+ii];
												amp1 = vector_time10[i][j];
												amp2 = vector_time10[neigh[i][3+ii]][jj];
												goto exit2;
											}
										}
									}
								}
							}
						}
					}
				}
exit2:                          ;
				if(trigger == 1) {
					//cout << event  << "\t" << "trig" << endl;
					vector <double> vector_event_amp;
					vector <int> vector_event_numb;
					vector <double> vector_event_amp1;
					vector <int> vector_event_numb1;
					for(int i = 0; i < (int)vector_phe_time.size(); i++) {
						if(vector_phe_time[i].size() != 0) {
							amp = 0;
							amp1 = 0;
							for(int j = 0; j < (int)vector_phe_time[i].size(); j++) { //идем по конкретным значениям времён прихода фотоэлектронов
								if(vector_phe_time[i][j] > trig_time - 1200 && vector_phe_time[i][j] <= trig_time + integrate_window) {
									amp = amp + vector_amp_MC[i][j]*inter.Eval(trig_time + integrate_window - vector_phe_time[i][j]);
									//cout << i << "\t" << vector_phe_time[i][j] << "\t" << vector_integral_hold[(int)rowcol[i][2]][ii]+35 << "\t" <<  vector_integral_hold[(int)rowcol[i][2]][ii]+35 - vector_phe_time[i][j] << "\tamp:\t" << inter.Eval(vector_integral_hold[(int)rowcol[i][2]][ii] + 35 - vector_phe_time[i][j]) << "\t" << amp << endl;
								}
								if(vector_phe_time[i][j] >= trig_time && vector_phe_time[i][j] <= (trig_time + integrate_window)) {
									amp1 = amp1 + vector_amp_MC[i][j];
								}
							}
							vector_event_amp.push_back(amp);
							vector_event_numb.push_back(i);

							vector_event_amp1.push_back(amp1);
							vector_event_numb1.push_back(i);
						}
					}
					if(vector_event_numb.size() > 0) {
							cout << evch << "\t" << read32[2] << "\t" << read32[0] << "\t" << read32[1] << "\t" << vector_event_amp.size() << "\t" << readdubble[0]/1E12 << "\t" << (readdubble[10] + readdubble[13])/1E3 << "\t " << (readdubble[11] + readdubble[14])/1E3 << endl;
							fout << read32[0] << "\t" << read32[1] << "\t" << vector_event_amp.size() << "\t" << readdubble[0]/1E12 << "\t" << (readdubble[10] + readdubble[13])/1E3 << "\t" << (readdubble[11] + readdubble[14])/1E3 << "\t" << tet_center << "\t" << fi_center << "\t" << tet_source << "\t" << fi_source << "\t" << readdubble[8] << endl;
							//fout1 << read32[0] << "\t" << read32[1] << "\t" << trig_clust+1  << "\t" << vector_event_amp.size() << "\t" << readdubble[0]/1E12 << "\t" << R << "\t"<< readdubble[17]*180/Pi << endl;
							for(int i = 0; i < (int)vector_event_numb.size(); i++) {
								//cout << neigh_clean[vector_event_numb[i]][1] << "\t" << neigh_clean[vector_event_numb[i]][2] << "\t" << neigh_clean[vector_event_numb[i]][3] << "\t"<< neigh_clean[vector_event_numb[i]][4] << "\t" << vector_event_amp[i] << endl;
								fout << real_numb[vector_event_numb[i]][0] << "\t" << real_numb[vector_event_numb[i]][1] << "\t" << coord[vector_event_numb[i]][0] << "\t"<< coord[vector_event_numb[i]][1] << "\t" << vector_event_amp[i] << endl;
								//fout1 << real_numb[vector_event_numb1[i]][1] << "\t" << real_numb[vector_event_numb1[i]][2] << "\t" << neigh_clean[vector_event_numb1[i]][3] << "\t"<< neigh_clean[vector_event_numb1[i]][4] << "\t" << vector_event_amp1[i] << endl;
							}
					}
					for(int i = 0; i < (int)vector_event_numb.size(); i++) {
						vector_event_amp.clear();
						vector_event_numb.clear();
						vector_event_amp1.clear();
						vector_event_numb1.clear();
					}
					evch++;
				}
			}
			for(int i = 0; i < (int)vector_phe_time.size(); i++) {
				vector_amp_MC[i].clear();
				vector_phe_time[i].clear();
				vector_amp[i].clear();
				vector_time10[i].clear();
				//test
				vector_integral35[i].clear();
			}
			for(int i = 0; i < (int)numb_of_clusters; i++) {
				vector_integral_hold[i].clear();
				vector_prelim_hold_time[i].clear();
				vector_prelim_hold_pix.clear();
			}
			//cout << "miss_phe " << miss_phe << endl;
			event++;
			/*if(event == ev + 2){
			   return 0;
			   }*/
		}
		else{
			fread(&readdubble, sizeof(double), 20, f5);
			for(int phe_i = 0; phe_i < read32[3]; phe_i++) {
				fread(&history, sizeof(int32_t), 1, f5);
				fread(&phe, sizeof(double), 2, f5); //time, ns and wavelength, nm
				fread(&roco, sizeof(int32_t), 2, f5);
			}
		}
	}
	return 0;
}
