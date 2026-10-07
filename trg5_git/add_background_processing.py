import pandas as pd
import numpy as np
import random
import multiprocessing
from tqdm import tqdm
import scipy.stats
import sys
#from matplotlib import pyplot as plt
#sys.path.append('/hdd/IACTs/Corsika/readbin/sim_stereo_analysis')
#import camera_display

def add_background(param_line):
    background_type = param_line[0]
    config_file_path = param_line[1]
    print(config_file_path)
    use_sens = param_line[2]
    factors_file_path = param_line[3]
    in_file = param_line[4]
    out_file = param_line[5]
    trigger_type = param_line[6]
    mean = param_line[7]
    std = param_line[8]
    n = int(param_line[9])
    background_paths = param_line[10]
    iact = int(param_line[11])
    save_img_path = param_line[12]
    coord_files = param_line[13]
    n_clusters = param_line[14] + 1
    n_channels = param_line[15]
    max_sense = param_line[16]
    factors = pd.read_csv(factors_file_path, sep='\s+').iloc[:].values
    sense = [[np.nan for x in range(n_channels)] for j in range(n_clusters)]
    if use_sens == 1:
        for factor in factors:
            if(int(factor[1])%2 == 0):
                sense[int(factor[0])][int(factor[1]/2)] = float(factor[3])
    if use_sens == 0:
        for factor in factors:
            if(int(factor[1])%2 == 0):
                sense[int(factor[0])][int(factor[1]/2)] = 1
    config_cam = pd.read_csv(config_file_path).iloc[:, [1,2,5,6]].values
    background_array = [[[] for x in range(n_channels)] for j in range(n_clusters)]
    sig_array = [[1000 for x in range(n_channels)] for j in range(n_clusters)]
    if(background_type == 0):
        for index in config_cam:
            background_array[int(index[0])][int(index[1])] = np.random.normal(mean, std, n)
    elif(background_type == 1):
        f = open(save_img_path, 'w')
        for background_path in background_paths:
            #print(background_path)
            with open(background_path) as file:
                while True:
                    line = file.readline()
                    if not line:
                        break
                    else:
                        cluster = int(line.split(',')[0])
                        pixel = int(line.split(',')[1])
                        arry = np.fromstring(line, dtype=float, sep=',')
                        background_array[cluster][pixel] = np.append(background_array[cluster][pixel], arry[2:-2])
                        #print(cluster, pixel, len(arry[2:-2]), len(background_array[cluster][pixel]))
                    #background_array[cluster][pixel] = background_array[cluster][pixel][background_array[cluster][pixel] > background_array[cluster][pixel].min()+0.2]
        for index in config_cam:
            cluster = int(index[0])
            pixel = int(index[1])
            #if(len(background_array[cluster][pixel])<50):
            if(len(background_array[cluster][pixel])<50 or abs(np.mean(background_array[cluster][pixel]))>10):
                background_array[cluster][pixel] = np.array([-1000])
                sig_array[cluster][pixel] = 1000
                #print(cluster,pixel)
                #clust_new = 0
                #while(len(background_array[cluster][pixel])<50 or abs(np.mean(background_array[cluster][pixel]))>10):
                #    background_array[cluster][pixel] = background_array[cluster-clust_new][2+pix_new]
                #    pix_new+=1
                #    if(pix_new > 28):
                #        clust_new+=1
                #        pix_new = 0
                #sig_array[cluster][pixel] = scipy.stats.median_abs_deviation(background_array[cluster][pixel], scale='normal')
                #background_array[cluster][pixel] = np.random.normal(mean, std, n)
                #background_array[cluster][pixel] = np.array([-10000,-10000])
                #sig_array[cluster][pixel] = 1000
                #if(cluster == 19):
                #    background_array[cluster][pixel] = background_array[18][7]
                #    sig_array[cluster][pixel] = np.median(abs(background_array[cluster][pixel]-np.median(background_array[cluster][pixel])))
            else:
                sig_array[cluster][pixel] = scipy.stats.median_abs_deviation(background_array[cluster][pixel])
                if(np.isnan(sig_array[cluster][pixel]) or sig_array[cluster][pixel] < 0.5):
                    background_array[cluster][pixel] = np.array([-1000])
                    sig_array[cluster][pixel] = 1000
            #background_array[cluster][pixel] = np.array(background_array[cluster][pixel])

        #pix_arr_std = []
        for index in config_cam:
            #pix_arr_std.append([int(index[0]),int(index[1]),sig_array[int(index[0])][int(index[1]/2)]])
            #camera_display.plot_image_only(iact-1, coord_files[iact-1],pix_arr_std, save_img_path)
            print(int(index[0]),int(index[1]),sig_array[int(index[0])][int(index[1])], file=f)

    f = open(out_file, 'w')
    file = open(in_file)
    for line in tqdm(file):
        if not line:
            break
        else:
            if trigger_type == 0:
                print(line[:-1], file=f)
            else:
                header = ''
                for i in range(len(line.split())):
                    if i != 2:
                        header = header + str(line.split()[i]) + '\t'
                    else:
                        header = header + str(len(config_cam)) + '\t'
                print(header, file=f)


            numb_pix = int(line.split()[2])
            camera_pixel = [[0 for x in range(n_channels)] for j in range(n_clusters)]
            for i in range(numb_pix):
                line = file.readline()
                cluster = int(line.split()[0])
                channel = int(line.split()[1])
                camera_pixel[cluster][channel] = float(line.split()[4])
                #array[cluster][channel] = np.array(array[cluster][channel])
                #print(len(array[cluster][channel][array[cluster][channel]>(-10)]))
                if trigger_type == 0:
                    x_cam = float(line.split()[2])
                    y_cam = float(line.split()[3])
                    #print(cluster, channel, background_array[cluster][channel])
                    background = random.choice(background_array[cluster][channel])
                    #background = 0
                    print(cluster,int(channel), x_cam, y_cam, round(camera_pixel[cluster][channel]/sense[cluster][channel] + background, 3), file = f)
            for index in config_cam:
                if trigger_type == 1:
                    background = random.choice(background_array[int(index[0])][int(index[1])])
                    print(int(index[0]),int(index[1]), index[2], index[3], round(camera_pixel[int(index[0])][int(index[1])]/sense[int(index[0])][int(index[1])] + background, 3), file = f)