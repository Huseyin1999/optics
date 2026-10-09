#include <iostream>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <locale>
#include <cstdlib>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#define Pi 3.141592653589793238462643
using namespace std;
const int n_clusters = 100;
const int max_n_channels_per_cluster = 32;
double x_pos[max_n_channels_per_cluster][n_clusters], y_pos[max_n_channels_per_cluster][n_clusters], bmp[max_n_channels_per_cluster][n_clusters], sig[max_n_channels_per_cluster][n_clusters], e[max_n_channels_per_cluster][n_clusters], sens[max_n_channels_per_cluster][n_clusters];
int j, jj, jjj, f;
int kkk[6][max_n_channels_per_cluster][n_clusters], pos[6][max_n_channels_per_cluster][n_clusters], islands[max_n_channels_per_cluster][n_clusters];
int number_of_pixels, numb_clean = 0, gam = 100;
double edge1 = 0.0, edge2 = 0.0, cm_to_deg = 0.1206;
string line, input_file_name, out_path, out_add;
vector<vector<double> > edge_pix(2);
double tet_center, fi_center, tet_source, fi_source;

double * get_hillas(vector<vector<double> > vector_pixel, double x_cam, double y_cam, int gam){
	static double hillas[21];
	double Xc[3], Yc[3], Xc2[3], Yc2[3], XYc[3], sigx2[3], sigy2[3], sigxy[3], azwidth[3], d[3], z[3], U[3], V[3], bm[3], miss[3], dist[3], alpha[3], a_axis[3], b_axis[3];
	//static double hillas[18]; //size, Xc[0],Yc[0], con2, length[0], width[0], dist[0], dist[1], dist[2], azwidth[0],
	//azwidth[1], azwidth[2], miss[0], miss[1], miss[2], alpha[0], alpha[1], alpha[2]
	//cout << "get hillas\t" << vector_pixel[0].size() << endl;
	double amp2 = 0;
	double amp_max = 0;
	double x_max_coord = 0;
	double y_max_coord = 0;
	double longitudinal[4] = { 0 };
	double latitudinal[4] = { 0 };
	double skewness_l = 0;
	double kurtosis_l = 0;
	double skewness_w = 0;
	double kurtosis_w = 0;
	double length_sig = 0;
	double width_sig = 0;
	double con3 = 0;
	double con2 = 0;
	double con1 = 0;
	double event_size = 0;
	for(int i = 0; i < 3; i++) {
		Xc[i] = 0;
		Yc[i] = 0;
		Xc2[i]=0;
		Yc2[i]=0;
		XYc[i]=0;
		sigx2[i] = 0;
		sigy2[i] = 0;
		sigxy[i] = 0;
		d[i]=0;
		z[i]=0;
		/*length[i]=0;
		width[i] = 0;*/
		azwidth[i]=0;
		U[i]=0;
		V[i]=0;
		bm[i]=0;
		miss[i]=0;
		dist[i]=0;
		alpha[i]=0;
		a_axis[i] = 0;
		b_axis[i] = 0;
	}
	for(int k = 0; k < vector_pixel[4].size(); k++) {
		Xc[0] += (vector_pixel[4][k]*(vector_pixel[2][k]));
		Yc[0] += (vector_pixel[4][k]*(vector_pixel[3][k]));
		Xc2[0] += (vector_pixel[4][k]*pow(vector_pixel[2][k], 2));
		Yc2[0] += (vector_pixel[4][k]*pow(vector_pixel[3][k], 2));
		XYc[0] += (vector_pixel[4][k]*(vector_pixel[2][k])*(vector_pixel[3][k]));

		Xc[1] += (vector_pixel[4][k]*(vector_pixel[2][k] - x_cam));
		Yc[1] += (vector_pixel[4][k]*(vector_pixel[3][k] - y_cam));
		Xc2[1] += (vector_pixel[4][k]*pow(vector_pixel[2][k] - x_cam, 2));
		Yc2[1] += (vector_pixel[4][k]*pow(vector_pixel[3][k] - y_cam, 2));
		XYc[1] += (vector_pixel[4][k]*(vector_pixel[2][k] - x_cam)*(vector_pixel[3][k] - y_cam));

		Xc[2] += (vector_pixel[4][k]*(vector_pixel[2][k] + x_cam));
		Yc[2] += (vector_pixel[4][k]*(vector_pixel[3][k] + y_cam));
		Xc2[2] += (vector_pixel[4][k]*pow(vector_pixel[2][k] + x_cam, 2));
		Yc2[2] += (vector_pixel[4][k]*pow(vector_pixel[3][k] + y_cam, 2));
		XYc[2] += (vector_pixel[4][k]*(vector_pixel[2][k] + x_cam)*(vector_pixel[3][k] + y_cam));

		if(amp_max <= vector_pixel[4][k]) {
			amp_max = vector_pixel[4][k];
			x_max_coord = vector_pixel[2][k];
			y_max_coord = vector_pixel[3][k];
		}
		event_size += vector_pixel[4][k];
	}
	vector<double> max_vec(3);
	partial_sort_copy(vector_pixel[4].begin(), vector_pixel[4].end(), max_vec.begin(), max_vec.end(), greater<double>());
	// con3 = (max_vec[0] + max_vec[1] + max_vec[2])/event_size;
	con2 = (max_vec[0] + max_vec[1])/event_size;
	// con1 = (max_vec[0])/event_size;

	for(int i = 0; i < 3; i++) {

		Xc[i] = Xc[i]/event_size;
		Yc[i] = Yc[i]/event_size;
		Xc2[i]=Xc2[i]/event_size;
		Yc2[i]=Yc2[i]/event_size;
		XYc[i]=XYc[i]/event_size;
		sigx2[i] = Xc2[i] - pow(Xc[i],2);
		sigy2[i] = Yc2[i] - pow(Yc[i],2);
		sigxy[i] = XYc[i] - (Xc[i])*(Yc[i]);
		d[i]=sigy2[i]-sigx2[i];
		a_axis[i] = (d[i] + sqrt(pow(d[i],2) + 4*pow(sigxy[i], 2)))/(2*sigxy[i]);
		b_axis[i] = Yc[i] - (a_axis[i]*Xc[i]);
		z[i]=sqrt(pow(d[i],2)+4.*pow(sigxy[i],2));
		/*length[i]=sqrt((sigx2[i] + sigy2[i] + z[i])/2.);
		if(sigx2[i] + sigy2[i] - z[i] >= 0) {
			width[i]=sqrt((sigx2[i] + sigy2[i] - z[i])/2.);
		}
		else{
			width[i] = 0;
		}*/
		dist[i] = sqrt(pow(Xc[i],2)+pow(Yc[i],2));
		azwidth[i] = sqrt((pow(Xc[i],2)*Yc2[i]-2.*Xc[i]*Yc[i]*XYc[i]+Xc2[i]*pow(Yc[i],2))/pow(dist[i],2));
		if(z[i] != 0) {
			U[i]=(1. + (d[i]/z[i]));
			V[i]=2. - U[i];
			bm[i] = (0.5*(U[i]*pow(Xc[i],2)+V[i]*pow(Yc[i],2)))-(2.*sigxy[i]*Xc[i]*Yc[i]/z[i]);
			if(bm[i] >= 0) {
				miss[i]=sqrt(bm[i]);
				alpha[i]=asin(miss[i]/dist[i])*(180./Pi);
			}
		}
	}
	for(int i = 0; i < 4; i++){
		for(int k = 0; k < vector_pixel[4].size(); k++) {
			longitudinal[i] += vector_pixel[4][k]*pow((vector_pixel[2][k] - Xc[0]) * cos(atan(a_axis[0])) + (vector_pixel[3][k] - Yc[0]) * sin(atan(a_axis[0])),i+1);
			latitudinal[i] += vector_pixel[4][k]*pow((vector_pixel[2][k] - Xc[0]) * cos(Pi/2 + atan(a_axis[0])) + (vector_pixel[3][k] - Yc[0]) * sin(Pi/2 + atan(a_axis[0])),i+1);
		}
		longitudinal[i] = longitudinal[i]/event_size;
		latitudinal[i] = latitudinal[i]/event_size;
	}
	length_sig = sqrt(longitudinal[1] - longitudinal[0] * longitudinal[0]);
	width_sig  = sqrt(latitudinal[1]  - latitudinal[0]  * latitudinal[0]);
	
	// Чтобы не делить на 0 (на всякий случай)
	const double eps = 1e-12;
	
	// ---- LONGITUDINAL ----
	if (length_sig > eps) {
		const double mu  = longitudinal[0];
		const double m2  = longitudinal[1];
		const double m3  = longitudinal[2];
		const double m4  = longitudinal[3];
	
		const double mu3 = m3 - 3.0 * mu * m2 + 2.0 * mu * mu * mu;                 // <-- 2*mu^3
		const double mu4 = m4 - 4.0 * mu * m3 + 6.0 * mu * mu * m2 - 3.0 * pow(mu,4); // центральный 4-й момент
	
		skewness_l = mu3 / pow(length_sig, 3);
		kurtosis_l = mu4 / pow(length_sig, 4);  // если нужен excess: kurtosis_l -= 3.0;
	} else {
		skewness_l = 0.0;
		kurtosis_l = 0.0;
	}
	
	// ---- LATITUDINAL ----
	if (width_sig > eps) {
		const double mu  = latitudinal[0];
		const double m2  = latitudinal[1];
		const double m3  = latitudinal[2];
		const double m4  = latitudinal[3];
	
		const double mu3 = m3 - 3.0 * mu * m2 + 2.0 * mu * mu * mu;                 // <-- 2*mu^3
		const double mu4 = m4 - 4.0 * mu * m3 + 6.0 * mu * mu * m2 - 3.0 * pow(mu,4);
	
		skewness_w = mu3 / pow(width_sig, 3);
		kurtosis_w = mu4 / pow(width_sig, 4);  // если нужен excess: kurtosis_w -= 3.0;
	} else {
		skewness_w = 0.0;
		kurtosis_w = 0.0;
	}
	
	// Твой блок со знаком skewness оставляю как есть (он про ориентацию)
	if ((gam == 1 && ((x_cam - Xc[0] < 0 && skewness_l > 0) || (x_cam - Xc[0] > 0 && skewness_l < 0))) ||
		(gam == 0 && ((x_cam > 0 && skewness_l < 0) || (x_cam < 0 && skewness_l > 0)))) {
		skewness_l = abs(skewness_l);
	} else {
		skewness_l = -abs(skewness_l);
	}
//gamma
	if(gam == 1) {
		hillas[0] = event_size;
		hillas[1] = cm_to_deg * Xc[0];
		hillas[2] = cm_to_deg * Yc[0];
		hillas[3] = con2;
		hillas[4] = cm_to_deg*length_sig;
		hillas[5] = cm_to_deg*width_sig;
		hillas[6] = cm_to_deg*dist[0];
		hillas[7] = cm_to_deg*dist[1];
		hillas[8] = cm_to_deg*dist[2];
		hillas[9] = cm_to_deg*azwidth[1];
		hillas[10] =  cm_to_deg*miss[1];
		hillas[11] = alpha[0];
		hillas[12] = alpha[1];
		hillas[13] = alpha[2];
		hillas[14] = a_axis[0];
		hillas[15] = b_axis[0];
		hillas[16] = cm_to_deg*x_max_coord;
		hillas[17] = cm_to_deg*y_max_coord;
		hillas[18] = skewness_l;
		hillas[19] = cm_to_deg*kurtosis_l;
		hillas[20] = skewness_w;
		hillas[21] = cm_to_deg*kurtosis_w;
		//hillas[20] = con1;
		//hillas[21] = con3;
	}
//hadron
	else{
		hillas[0] = event_size;
		hillas[1] = cm_to_deg * Xc[0];
		hillas[2] = cm_to_deg*Yc[0];
		hillas[3] = con2;
		hillas[4] = cm_to_deg*length_sig;
		hillas[5] = cm_to_deg*width_sig;
		hillas[6] = cm_to_deg*dist[0];
		hillas[7] = cm_to_deg*dist[0];
		hillas[8] = cm_to_deg*dist[0];
		hillas[9] = cm_to_deg*azwidth[0];
		hillas[10] =  cm_to_deg*miss[0];
		hillas[11] = alpha[0];
		hillas[12] = alpha[0];
		hillas[13] = alpha[0];
		hillas[14] = a_axis[0];
		hillas[15] = b_axis[0];
		hillas[16] = cm_to_deg*x_max_coord;
		hillas[17] = cm_to_deg*y_max_coord;
		hillas[18] = skewness_l;
		hillas[19] = cm_to_deg*kurtosis_l;
		hillas[20] = skewness_w;
		hillas[21] = cm_to_deg*kurtosis_w;
		//hillas[20] = con1;
		//hillas[21] = con3;
	}
	return hillas;
}

void get_calibation(string factor_file){
	ifstream file0(factor_file);
	getline(file0, line);
	int bsm,cch;
	double ecode, rel_sens;
	if (!file0.is_open()) {
		cout << "calibration file is not found" << endl;
		exit(0);
	}
	while(!file0.eof()) {
		getline(file0, line);
		if(!file0.eof()) {
			istringstream ist(line);
			ist >> bsm >> cch >> ecode >> rel_sens;
			if(cch%2 == 0){
				if(ecode > 0) {
					e[(int)cch/2][bsm-1] = ecode;
					sens[(int)cch/2][bsm-1] = rel_sens;
				}
				else{
					e[(int)cch/2][bsm-1] = 1e9;
					sens[(int)cch/2][bsm-1] = -1e9;
				}
			}
		}
	}
}

void get_experimental_sigma(string file_path, int cleaning_type){
	ifstream DataFilePeds;
	int ff, ch;
	double pedp, sigp;
	for (int count = 0; count < n_clusters; count++)
	{
		for (int coun = 0; coun < max_n_channels_per_cluster; coun++)
		{
			sig[coun][count] = 1;
		}
	}
	if(cleaning_type == 1) {
		DataFilePeds.open(file_path);
		if (DataFilePeds.is_open()) {
			//cout << cleaning_type << endl;
			while (!DataFilePeds.eof())
			{
				getline(DataFilePeds, line);
				istringstream iss(line);
				sigp=-40;
				pedp=-40;
				iss >> ff >> ch >> pedp >> sigp;
				if(ff>(n_clusters-1)){
					break;
				}
				if(ch%2 == 0){
					if(e[(int)ch/2][ff-1] > 0 && sens[(int)ch/2][ff-1] > 0 && sigp > 0)
						sig[(int)ch/2][ff-1]=sigp/(e[(int)ch/2][ff-1]*sens[(int)ch/2][ff-1]);
						//cout << ff << "\t" << (int)ch/2 << "\t" << sig[(int)ch/2][ff-1] << endl;
					cout << ff << "\t" << ch/2 << "\t" << sig[(int)ch/2][ff - 1] << endl;
				}
			}
			DataFilePeds.close();
		}
	}
	else{
		cout << "number of clusters: " << n_clusters << "\tmax number of channels per pixel: " << max_n_channels_per_cluster << "\tsigma = 1" << endl;
	}
}

void read_neigbours(int iact_i, int number_of_pixels, string config_file){
	for (int count = 0; count < n_clusters; count++)
	{
		for (int coun = 0; coun < max_n_channels_per_cluster; coun++)
		{
			for (int cou = 0; cou < 6; cou++)
			{
				kkk[cou][coun][count] = -1;
				pos[cou][coun][count] = -1;
			}
		}
	}
	char notification[150];
	ifstream file(config_file);
	if (!file.is_open()) {
		sprintf(notification, "%s%d%s", "IACT", iact_i, "_cam_corsica_config Файл не найден");
		cout << notification << endl;
	}
	else {
		string source;
		int pix_numb;
		int neigh[number_of_pixels][10];
		getline(file, line);
		for(int ii = 0; ii < number_of_pixels; ii++) {
			getline(file, line);
			stringstream ist(line);
			for(int i = 0; i < 16; i++) {
				getline(ist, source, ',');
				if(i==0) {//pix_numb
					pix_numb=atoi(source.c_str());
				}
				if(i==1) {//cluster
					neigh[pix_numb][0]=atoi(source.c_str());
					//cout << neigh[pix_numb][0] << "\t";
				}
				if(i==2) {//channel
					neigh[pix_numb][1]=atoi(source.c_str());
					//cout << neigh[pix_numb][1] << endl;
				}
				if(i==5) {//x_cam
					x_pos[neigh[pix_numb][1]][neigh[pix_numb][0]-1]=atof(source.c_str());
				}
				if(i==6) {//y_cam
					y_pos[neigh[pix_numb][1]][neigh[pix_numb][0]-1]=atof(source.c_str());
				}
				if(i==9) {//n_neighbours
					neigh[pix_numb][2]=atoi(source.c_str());
				}
				if(i>=10) {//neighbour_index
					neigh[pix_numb][3+i-10]=atoi(source.c_str());
				}
			}
		}
		for(int ii = 0; ii < number_of_pixels; ii++) {
			//cout << neigh[ii][0] << "\t" << neigh[ii][1] << "\t\t";
			for(int j=0; j<neigh[ii][2]; j++){
				//cout << ii << "\t" << 3+j << "\t" << neigh[ii][3+j] << "\t" << neigh[neigh[ii][3+j]][0] - 1 << "\t" << neigh[neigh[ii][3+j]][1] << endl;
				kkk[j][neigh[ii][1]][neigh[ii][0]-1] = neigh[neigh[ii][3+j]][0] - 1;
				pos[j][neigh[ii][1]][neigh[ii][0]-1] = neigh[neigh[ii][3+j]][1];
			}
			if(neigh[ii][2] < 6){
				edge_pix[0].push_back(neigh[ii][0]-1);
				edge_pix[1].push_back(neigh[ii][1]);
			}
			//for(int j=0; j<neigh[ii][2]; j++){
			//	cout <<  kkk[j][neigh[ii][1]][neigh[ii][0]-1]+1 << "\t" << pos[j][neigh[ii][1]][neigh[ii][0]-1] << "\t";
			//}
			//cout << endl;
		}
	}
	/*for (int count = 0; count < n_clusters; count++)
        {
            for (int coun = 0; coun < max_n_channels_per_cluster; coun++)
            {
				cout << count << "\t" << coun << "\t";
				for(int i=0; i<6;i++){
                	 cout << kkk[i][coun][count] << "\t" << pos[i][coun][count] << "\t";
				}
				cout << endl;
            }
		exit(0);
        }*/
	file.close();
}

void get_islands(double bmp[max_n_channels_per_cluster][n_clusters], int kkk[6][max_n_channels_per_cluster][n_clusters], int pos[6][max_n_channels_per_cluster][n_clusters], int islands[max_n_channels_per_cluster][n_clusters]){
    int island = 0;
    int visited[max_n_channels_per_cluster][n_clusters];
    vector <int> vclust;
    vector <int> vpix;

    for (int count = 0; count < n_clusters; count++)
        {
            for (int coun = 0; coun < max_n_channels_per_cluster; coun++)
            {
                visited[coun][count] = 0;
                islands[coun][count] = 0;
            }
        }

    for (int count = 0; count < n_clusters; count++)
        {
            for (int coun = 0; coun < max_n_channels_per_cluster; coun++)
            {
                if(bmp[coun][count] == 0 || visited[coun][count] == 1){
                    continue;
                }
                visited[coun][count] = 1;
                island++;
                islands[coun][count] = island;
                for(int i = 0; i < 6; i++){
                    if(bmp[pos[i][coun][count]][kkk[i][coun][count]] == 0 ||
                    visited[pos[i][coun][count]][kkk[i][coun][count]] == 1 ||
                    (pos[i][coun][count] == 0 && kkk[i][coun][count] == 0)){
                        continue;
                    }
                    //cout << count << "\t" << coun << "\t" << kkk[i][coun][count] << "\t" << pos[i][coun][count] << endl;
                    visited[pos[i][coun][count]][kkk[i][coun][count]] = 1;
                    islands[pos[i][coun][count]][kkk[i][coun][count]] = island;
                    vclust.push_back(kkk[i][coun][count]);
                    vpix.push_back(pos[i][coun][count]);
                }
				int added[max_n_channels_per_cluster][n_clusters];
				for (int countt = 0; countt < n_clusters; countt++)
				{
					for (int cou = 0; cou < max_n_channels_per_cluster; cou++)
					{
						added[cou][countt] = 0;
					}
				}
                while(vclust.size() > 0){
                    //cout << vclust.size() << "\t" << vclust[0] << "\t" << vpix[0] << endl;
                    for(int i = 0; i < 6; i++){
                        //cout << kkk[i][vpix[0]][vclust[0]] << "\t" << pos[i][vpix[0]][vclust[0]] << "\t" << bmp[pos[i][vpix[0]][vclust[0]]][kkk[i][vpix[0]][vclust[0]]] << "\t" << visited[pos[i][vpix[0]][vclust[0]]][kkk[i][vpix[0]][vclust[0]]] << endl;
                        if(bmp[pos[i][vpix[0]][vclust[0]]][kkk[i][vpix[0]][vclust[0]]] == 0 ||
                        visited[pos[i][vpix[0]][vclust[0]]][kkk[i][vpix[0]][vclust[0]]] == 1 ||
                        (pos[i][vpix[0]][vclust[0]] == 0 && kkk[i][vpix[0]][vclust[0]] == 0) ||
						added[pos[i][vpix[0]][vclust[0]]][kkk[i][vpix[0]][vclust[0]]] == 1){
                            continue;
                        }
                        vclust.push_back(kkk[i][vpix[0]][vclust[0]]);
                        vpix.push_back(pos[i][vpix[0]][vclust[0]]);
						added[pos[i][vpix[0]][vclust[0]]][kkk[i][vpix[0]][vclust[0]]] = 1;
                    }
                    visited[vpix[0]][vclust[0]] = 1;
                    islands[vpix[0]][vclust[0]] = island;
					/*for(int i = 0; i < vclust.size(); i++){
						cout << vclust.size() << "\t" << vclust[i] << "\t" << vpix[i] << "\t" << visited[vpix[i]][vclust[i]] << endl;
					}*/
                    vclust.erase(vclust.begin());
                    vpix.erase(vpix.begin());
                }
        }
    }
    islands[0][0] = island;
}

double get_bightest_island(double bmp[max_n_channels_per_cluster][n_clusters], int islands[max_n_channels_per_cluster][n_clusters]){
    int islands_numb = islands[0][0];
    double size_islands[islands_numb];
    for(int i = 0; i < islands_numb; i++){
        size_islands[i] = 0;
    }
    for(int coun = 0; coun < n_clusters; coun++) {
		for(int count = 0; count < max_n_channels_per_cluster; count++) {
            if(bmp[count][coun] > 0){
                size_islands[islands[count][coun]-1]+=bmp[count][coun];
            }
        }
    }
    int brightest_island = 0;
    double brightest_island_size = 0;
	double sum_size = 0;
    for(int i = 0; i < islands_numb; i++){
        sum_size+=size_islands[i];
        if(size_islands[i] > brightest_island_size){
			brightest_island_size = size_islands[i];
            brightest_island = i+1;

        }
    }
    //cout << endl;
    for(int coun = 0; coun < n_clusters; coun++) {
		for(int count = 0; count < max_n_channels_per_cluster; count++) {
            if(islands[count][coun] != brightest_island){
                bmp[count][coun] = 0;
            }
        }
    }
	//cout << brightest_island_size << "\t" << sum_size << endl;
	return brightest_island_size/sum_size;
}

vector<vector<double> > cleaning(int use_brightest_island, double edge1, double edge2, double bmp[max_n_channels_per_cluster][n_clusters]){
	int amp_counter = 0;
	for(f = 0; f < (n_clusters-1); f++)
	{
		//cout << f << endl;
		jj = 0;
		jjj = 0;
		for (int sc = 0; sc < max_n_channels_per_cluster; sc++)
		{
			if(bmp[sc][f] > 0) {
				jj++;
			}
			if(bmp[sc][f] <= 0) {
				bmp[sc][f] = 0;
				jjj++;
			}
		}
		//cout << jj << endl;
		if( jj > 0) {
			for (int sc = 0; sc < max_n_channels_per_cluster; sc++)
			{
				//cout << sc << "\t" << f << "\t" << bmp[sc][f] << "\t" << pos[1][sc][f] << "\t" << kkk[1][sc][f] << "\t" << bmp[sc][f] << "\t" << bmp[pos[1][sc][f]][kkk[1][sc][f]] << endl;
				if(bmp[sc][f]>edge1*sig[sc][f])
				{
					//cout << pos[0][sc][f] << "\t" << kkk[0][sc][f] << endl;
					if((bmp[pos[0][sc][f]][kkk[0][sc][f]]>edge2*sig[pos[0][sc][f]][kkk[0][sc][f]] && bmp[pos[0][sc][f]][kkk[0][sc][f]]>0) ||
					   (bmp[pos[1][sc][f]][kkk[1][sc][f]]>edge2*sig[pos[1][sc][f]][kkk[1][sc][f]] && bmp[pos[1][sc][f]][kkk[1][sc][f]]>0) ||
					   (bmp[pos[2][sc][f]][kkk[2][sc][f]]>edge2*sig[pos[2][sc][f]][kkk[2][sc][f]] && bmp[pos[2][sc][f]][kkk[2][sc][f]]>0) ||
					   (bmp[pos[3][sc][f]][kkk[3][sc][f]]>edge2*sig[pos[3][sc][f]][kkk[3][sc][f]] && bmp[pos[3][sc][f]][kkk[3][sc][f]]>0) ||
					   (bmp[pos[4][sc][f]][kkk[4][sc][f]]>edge2*sig[pos[4][sc][f]][kkk[4][sc][f]] && bmp[pos[4][sc][f]][kkk[4][sc][f]]>0) ||
					   (bmp[pos[5][sc][f]][kkk[5][sc][f]]>edge2*sig[pos[5][sc][f]][kkk[5][sc][f]] && bmp[pos[5][sc][f]][kkk[5][sc][f]]>0))
					{
						amp_counter = 1;
					}
					else
					{
						bmp[sc][f] = 0;
					}
				}
				else if(bmp[sc][f]>edge2*sig[sc][f])
				{
					if((bmp[pos[0][sc][f]][kkk[0][sc][f]]>edge1*sig[pos[0][sc][f]][kkk[0][sc][f]] && bmp[pos[0][sc][f]][kkk[0][sc][f]]>0) ||
					   (bmp[pos[1][sc][f]][kkk[1][sc][f]]>edge1*sig[pos[1][sc][f]][kkk[1][sc][f]] && bmp[pos[1][sc][f]][kkk[1][sc][f]]>0) ||
					   (bmp[pos[2][sc][f]][kkk[2][sc][f]]>edge1*sig[pos[2][sc][f]][kkk[2][sc][f]] && bmp[pos[2][sc][f]][kkk[2][sc][f]]>0) ||
					   (bmp[pos[3][sc][f]][kkk[3][sc][f]]>edge1*sig[pos[3][sc][f]][kkk[3][sc][f]] && bmp[pos[3][sc][f]][kkk[3][sc][f]]>0) ||
					   (bmp[pos[4][sc][f]][kkk[4][sc][f]]>edge1*sig[pos[4][sc][f]][kkk[4][sc][f]] && bmp[pos[4][sc][f]][kkk[4][sc][f]]>0) ||
					   (bmp[pos[5][sc][f]][kkk[5][sc][f]]>edge1*sig[pos[5][sc][f]][kkk[5][sc][f]] && bmp[pos[5][sc][f]][kkk[5][sc][f]]>0))
					{
						amp_counter = 1;
					}
					else
					{
						bmp[sc][f] = 0;
					}
				}
				else
				{
					bmp[sc][f] = 0;
				}
			}
			j = 0;
			for (int sc = 0; sc < max_n_channels_per_cluster; sc++)
			{
				if(bmp[sc][f] == 0)
				{
					j++;
				}
			}
		}
	}
	vector<vector<double> > vector_pixel( 6, vector<double> (0));
	if(amp_counter == 1){
		get_islands(bmp, kkk, pos, islands);
		if(use_brightest_island == 1){
    		vector_pixel[5].push_back(get_bightest_island(bmp, islands));
			}
		else{
			vector_pixel[5].push_back(1);
		}

		for (int count = 0; count < n_clusters; count++)
		{
			for (int coun = 0; coun < max_n_channels_per_cluster; coun++)
			{
				if(bmp[coun][count] != 0) {
					number_of_pixels++;
					vector_pixel[0].push_back(count+1);
					vector_pixel[1].push_back(coun);
					vector_pixel[4].push_back(bmp[coun][count]);
					vector_pixel[2].push_back(x_pos[coun][count]);
					vector_pixel[3].push_back(y_pos[coun][count]);
				}
			}
		}
	}
	return vector_pixel;
}

int main(int argc, char **argv){
	//cout << argv[1] << endl;
	srand(time(0));
	int i_iact = atoi(argv[1]);
	gam = atoi(argv[2]);
	int n_pixels = atoi(argv[3]);
	edge1 = atof(argv[4]);
	edge2 = atof(argv[5]);
	int use_brightest_island = atoi(argv[14]);
	numb_clean = 0;
	read_neigbours(i_iact, n_pixels, argv[6]);
	double scatter_cam_radius = atof(argv[10]);
	char str_line[200];
	ifstream file(argv[7]);//in file
	ofstream fout(argv[8]);//out clean images
	ofstream fout1(argv[9]);//out hillas table
	fout1 << "clean_number,event_number,axis_scatter,numb_pix,energy,size,Xc,Yc,con2,length,width,dist0,dist1,dist2,azwidth,miss,alpha0,alpha1,alpha2,a_axis,b_axis,x_max_coord,y_max_coord,skewness_l,kurtosis_l,skewness_w,kurtosis_w,edge,source_x,source_y,x_ground,y_ground,tel_tet,tel_fi,source_tet,source_fi,Xmax,num_islands,con_selected_island" << endl;
	if (!file.is_open()) {
		cout << "data файл не найден" << endl;
	}
	else {
		for (int coun = 0; coun < n_clusters; coun++)
		{
			for (int count = 0; count < max_n_channels_per_cluster; count++)
			{
				sig[count][coun] = 10000;
			}
		}
		get_calibation(argv[11]);
		get_experimental_sigma(argv[12], atoi(argv[13]));
		while(!file.eof()) {
			getline(file, line);
			//cout << line << endl;
			if(!file.eof()) {
				int numb = 0, i_scat = 0, number_of_pixels = 0;
				double con_brightest_island = 1.0, edge = 0;
				double energy = 0, distance = 0, angle = 0, x_cam = 0, y_cam = 0, x_ground = 0, y_ground = 0, xmax = 0;
				for (int count = 0; count < n_clusters; count++)
				{
					for (int coun = 0; coun < max_n_channels_per_cluster; coun++)
					{
						y_pos[coun][count]=0;
						x_pos[coun][count]=0;
						bmp[coun][count]=0;
					}
				}
				stringstream ist(line);
				//ist >> numb >> i_scat >> clust >> number_of_pixels >> energy >> distance >> angle;
				ist >> numb >> i_scat >> number_of_pixels >> energy >> x_ground >> y_ground >> tet_center >> fi_center >> tet_source >> fi_source >> xmax;

				//for optic version >=31:
				fi_source = M_PI - fi_source;
				fi_center = M_PI - fi_center;

				/*double xToSource=1*sin(tet_source)*cos(fi_source);
				double yToSource=1*sin(tet_source)*sin(fi_source);
				double zToSource=1*cos(tet_source);

				double xToCenter=1*sin(tet_center)*cos(-fi_center);
				double yToCenter=1*sin(tet_center)*sin(-fi_center);
				double zToCenter=1*cos(tet_center);

				double thetamir_c= (180./Pi)*acos((xToSource*xToCenter+yToSource*yToCenter+zToSource*zToCenter));
				double axis[3] = {-sin(fi_center), cos(fi_center), 0};

				double M[3][3]={{(1-cos(tet_center))*axis[0]*axis[0]+cos(tet_center),    (1-cos(tet_center))*axis[1]*axis[0]+sin(tet_center)*axis[2],  (1-cos(tet_center))*axis[2]*axis[0]-sin(tet_center)*axis[1]},
					{(1-cos(tet_center))*axis[0]*axis[1]-sin(tet_center)*axis[2], (1-cos(tet_center))*axis[1]*axis[1]+cos(tet_center),     (1-cos(tet_center))*axis[2]*axis[1]+sin(tet_center)*axis[0]},
					{(1-cos(tet_center))*axis[0]*axis[2]+sin(tet_center)*axis[1],  (1-cos(tet_center))*axis[1]*axis[2]- sin(tet_center)*axis[0], (1-cos(tet_center))*axis[2]*axis[2]+cos(tet_center)}};

				double Ptheta[3]={axis[1]*sin(tet_center), -axis[0]*sin(tet_center), cos(tet_center)};
				double XYZmir_c[3];
				for(int i=0; i<3; i++) {
					xToSource=1*sin(tet_source)*cos(-fi_source);
					yToSource=1*sin(tet_source)*sin(-fi_source);
					zToSource=1*cos(tet_source);
					XYZmir_c[i] = M[i][0]*xToSource + M[i][1]*yToSource + M[i][2]*zToSource;
				}
				double phimiratan2_c = (180./Pi)*atan2((XYZmir_c[1]),(XYZmir_c[0]));
				double XToSourcecamatan2=(180./Pi)*tan(thetamir_c*Pi/180.)*cos((phimiratan2_c + 180)*Pi/180.);
				double YToSourcecamatan2=(180./Pi)*tan(thetamir_c*Pi/180.)*sin((phimiratan2_c + 180)*Pi/180.);*/

				//double XToSourcecamatan2_cm = (XToSourcecamatan2/(180./(475.*Pi)));
				//double YToSourcecamatan2_cm = (YToSourcecamatan2/(180./(475.*Pi)));

				double XToSourcecamatan2_cm = ((475./(sin(tet_source)*sin(tet_center) + cos(tet_source)*cos(tet_center)*cos(fi_source-fi_center)))*(cos(tet_source)*sin(fi_source-fi_center))*tan(tet_source));
				double YToSourcecamatan2_cm = ((475./(sin(tet_source)*sin(tet_center) + cos(tet_source)*cos(tet_center)*cos(fi_source-fi_center)))*(cos(tet_source)*sin(tet_center)*cos(fi_source - fi_center) - sin(tet_source)*cos(tet_center)));
				if(gam == 0){
					double rxy = sqrt(pow(scatter_cam_radius/cm_to_deg,2) + pow(scatter_cam_radius/cm_to_deg,2));
					double xr = 0;
					double yr = 0;
					while(rxy > scatter_cam_radius/cm_to_deg){
						xr = (((double)rand() / RAND_MAX) * (scatter_cam_radius/cm_to_deg + scatter_cam_radius/cm_to_deg)) - scatter_cam_radius/cm_to_deg;
						yr = (((double)rand() / RAND_MAX) * (scatter_cam_radius/cm_to_deg + scatter_cam_radius/cm_to_deg)) - scatter_cam_radius/cm_to_deg;
						rxy = sqrt(pow(xr,2) + pow(yr,2));
					}
					//cout << vr << "\t" << vtet << endl;
					XToSourcecamatan2_cm = xr;
					YToSourcecamatan2_cm = yr;
				}
				//cout << tet_source << "\t" << tet_center << "\t" << fi_source << "\t" << fi_center << "\t" << XToSourcecamatan2_cm << "\t" << YToSourcecamatan2_cm << endl;
				for(int i = 0; i < number_of_pixels; i++) {
					int channel = 0, cluster = 0;
					getline(file, line);
					stringstream ist(line);
					ist >> cluster >> channel;
					ist >> x_pos[channel][cluster-1] >> y_pos[channel][cluster-1] >> bmp[channel][cluster-1];
					//cout << bmp[channel][cluster-1] << endl;
					//bmp[channel][cluster-1] = bmp[channel][cluster-1];
				}
				vector<vector<double> > vector_pixel( 6, vector<double> (0));
				cout << numb << '\t' << i_scat << endl;
				vector_pixel = cleaning(use_brightest_island, edge1, edge2, bmp);
				cout << numb << '\t' << i_scat << endl;
				//cout << vector_pixel[0].size() << endl;
				if(vector_pixel[0].size() > 3) {
					double *hillas;
					//cout << vector_pixel[0].size() << endl;
					hillas = get_hillas(vector_pixel, XToSourcecamatan2_cm, YToSourcecamatan2_cm, 1);//передаем gam=1, потому что уже заменили координаты источника на случайные в том же круге что и для гамма
					fout << setw(6);
					fout1 << setw(0);
					fout << numb_clean << setw(7) << numb << setw(6) << i_scat << setw(6) << vector_pixel[0].size() << setw(13) << energy << setw(13) << XToSourcecamatan2_cm << setw(13) << YToSourcecamatan2_cm << setw(13) << x_ground << setw(13) << y_ground << setw(13) << hillas[14] << setw(13) << hillas[15] << endl;
					for(int i = 0; i < vector_pixel[0].size(); i++) {
						for(int kk = 0; kk < edge_pix[0].size(); kk++) {
							if(vector_pixel[0][i] == edge_pix[0][kk]) {
								if(vector_pixel[1][i] == edge_pix[1][kk]) {
									edge = edge + vector_pixel[4][i];
								}
							}
						}
						fout << setw(0);
						for(int ii = 0; ii < 5; ii++) {
							fout << vector_pixel[ii][i] << setw(10);
						}
						fout << endl;
					}
					edge = edge/hillas[0];
					fout1 << numb_clean << ',' << numb << ',' << i_scat << ',' << vector_pixel[0].size() << ',' << energy << ',';
					for(int i = 0; i < 22; i++) {
						fout1 <<  hillas[i] << ',';
					}
					fout1 << edge << ',' << XToSourcecamatan2_cm*cm_to_deg << ',' << YToSourcecamatan2_cm*cm_to_deg << ',' << x_ground << ',' << y_ground << ',' << tet_center << ',' << fi_center << ',' << tet_source << ',' << fi_source << ',' << xmax  << "," << islands[0][0] << "," << vector_pixel[5][0] << endl;
					numb_clean++;
				}
			}
		}
	}
	file.close();
	//cout << edge_pix[0].size() << endl;
	edge_pix[0].clear();
	edge_pix[1].clear();
	return 0;
}
