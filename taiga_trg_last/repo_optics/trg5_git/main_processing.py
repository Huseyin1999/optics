# main.py

import sys
import json
import os, re, math, shutil, multiprocessing
import pandas as pd
import numpy as np
from add_background_processing import add_background

def rotate(point, angle, origin = [0,0]):
    """
    Rotate a point counterclockwise by a given angle around a given origin.

    The angle should be given in radians.
    """
    ox, oy = origin
    px, py = point

    qx = ox + math.cos(angle) * (px - ox) - math.sin(angle) * (py - oy)
    qy = oy + math.sin(angle) * (px - ox) + math.cos(angle) * (py - oy)
    return qx, qy

def get_config(IACT_numb, path_coord, path_sense, use_sense, save_path):
    exp_coord = pd.read_csv(path_coord, header = None, sep = r'\s+', usecols = [0,3,4,7])
    exp_sense = pd.read_csv(path_sense, sep = r'\s+', usecols = [0,1,3])
    max_sense = exp_sense['Rel_sens'].max()
    s_pmt = 30
    rot_angle = 37.5
    #rot_angle = 0
    cm_to_deg = 0.1206
    trig_circle_deg = 3.7
    pixel_cm_width = 3.0
    columns_table = ['pixel_number', 'cluster', 'channel', 'row', 'col', 'x_cam', 'y_cam', 'triger', 'sense', 'n_cluster_neighbours', '1', '2', '3', '4', '5', '6']
    table = pd.DataFrame(columns = columns_table)
    for i,value in enumerate(zip(exp_coord[3],exp_coord[4])):

        coord_corsika= rotate(value, rot_angle*np.pi/180)
        Nr = 2*coord_corsika[1]*10/(s_pmt*np.sqrt(3))
        Nc = ((2*coord_corsika[0]*10/s_pmt) + round(Nr)%2)/2

        #использовать для нахождения соответствия координат реальных и в оптике
        pmt_x = ((s_pmt/2)*(2*round(Nc) - round(Nr)%2))/10
        pmt_y = round(Nr)*s_pmt*np.sqrt(3)/20

        #использовать если в оптике камера уже повернута на -37.5
        #pmt_x = value[0]
        #pmt_y = value[1]

        neigh_arr = []
        distance0 = cm_to_deg*(((value[0])** 2  +  (value[1]) ** 2) ** 0.5)
        if distance0 < trig_circle_deg:
            trig = 1
        else:
            trig = 0
        for j,j_value in enumerate(zip(exp_coord[3],exp_coord[4])):
            distance = ((j_value[0] - value[0])** 2  +  (j_value[1] - value[1]) ** 2) ** 0.5
            if distance < (pixel_cm_width + 0.5) and distance > (pixel_cm_width - 0.5):
                neigh_arr.append(j)
            n_neigh = len(neigh_arr)
        while len(neigh_arr) < 6:
            neigh_arr.append(-1)
        #print(neigh_arr)
        if use_sense == 1:
            sense = exp_sense['Rel_sens'][(exp_sense['Cluster'] == exp_coord[0][i]) & (exp_sense['Channel']/2 == exp_coord[7][i]) & (exp_sense['Channel']%2 == 0)].values[0]/max_sense
        else:
            sense = 1.0
        #print(i, exp_coord[0][i], exp_coord[7][i], round(Nr), round(Nc), pmt_x, pmt_y, trig, sense, n_neigh, *neigh_arr)
        table.loc[i] = [i, exp_coord[0][i], exp_coord[7][i], round(Nr), round(Nc), pmt_x, pmt_y, trig, sense, n_neigh, *neigh_arr]
    for col in columns_table:
        if col!= 'x_cam' and  col!='y_cam' and col!='sense':
            table[col] = pd.to_numeric(table[col], downcast='integer')
    config_path = save_path + '/IACT0' + str(IACT_numb) + '_cam_corsica_config.csv'
    table.to_csv(config_path, index = False)
    return len(exp_coord), exp_coord[0].iloc[-1], config_path, max_sense

def get_slow_pulse_const(path):
    pulse_array = pd.read_csv(path, header = None, sep = r'\s+')
    return(pulse_array[0].iloc[0], pulse_array[1].max(), len(pulse_array[0]))

def launch_gcc_script(params):
    script_name = params[0]
    os.system('./' + script_name + ' ' + ('{} '*len(params[1:])).format(*params[1:]))

def uniquify(path):
    counter = 1
    new_path = path
    while os.path.exists(new_path):
        new_path = f"{path}_{counter}"
        counter += 1
    return new_path

def main():
    # Проверяем, что передали два аргумента
    if len(sys.argv) != 3:
        print(f"Использование: python {os.path.basename(sys.argv[0])} <config.json> <initial_file>")
        sys.exit(1)

    config_path = sys.argv[1]
    initial_file = sys.argv[2]

    # 1) Читаем JSON-конфиг
    with open(config_path, 'r', encoding='utf-8') as f:
        cfg = json.load(f)

    # Если background_params — список, оставляем; если словарь со строковыми ключами, конвертируем:
    if isinstance(cfg.get("background_params"), dict):
        cfg["background_params"] = {
            int(k): v for k, v in cfg["background_params"].items()
        }
        # Преобразуем ключи во вложенных словарях
        for v in cfg["background_params"].values():
            if isinstance(v, dict) and 'distribution background' in v:
                v['distribution background'] = {
                    int(k): val for k, val in v['distribution background'].items()
                }
            if isinstance(v, dict) and 'experimental_background_path' in v:
                v['experimental_background_path'] = {
                    int(k): val for k, val in v['experimental_background_path'].items()
                }
    # 2) Вычисляем динамические параметры
    run_i = int(re.findall(r'\d+', initial_file)[-1])
    initial_dir = os.path.dirname(initial_file)
    folder_name = os.path.basename(os.path.normpath(initial_dir)) + '_trg5'
    file_simulation = f"taiga{run_i}_feb"

    # 3) Формируем save_path0
    save_path0 = cfg["save_path"] + '/' + folder_name + "/" + cfg["sim_name"]

    # 4) Распаковываем все параметры из cfg
    IACTs = cfg["IACTs"]
    pulse_files = cfg["pulse_files"]
    coord_files = cfg["coord_files"]
    factors_path = cfg["factors_path"]
    trigger_type = cfg["trigger_type"]
    integrate_window = cfg["integrate_window"]
    gam = cfg["gam"]
    use_sense = cfg["use_sense"]
    amplitudes_file = cfg["amplitudes_file"]
    source_scatter_radius = cfg["source_scatter_radius"]
    t_grid = cfg["t_grid"]
    t_sign = cfg["t_sign"]
    trig_amp = cfg["trig_amp"]
    t_hold = cfg["t_hold"]
    trig_window = cfg["trig_window"]
    t_after_before = cfg["t_after_before"]
    sense_cam = cfg["sense_cam"]
    max_n_channels_per_cluster = cfg["max_n_channels_per_cluster"]
    hidden_mirror = cfg["hidden_mirror"]
    background = cfg["background"]
    background_params = cfg["background_params"]
    sigma_for_clean = cfg["sigma_for_clean"]
    clean_edge1 = cfg["clean_edge1"]
    clean_edge2 = cfg["clean_edge2"]
    use_sigma_for_cleaning = cfg["use_sigma_for_cleaning"]
    use_brightest_island = cfg["use_brightest_island"]
    make_trigger = cfg["make_trigger"]
    make_background = cfg["make_background"]
    make_cleaning = cfg["make_cleaning"][0]
    background_folder_for_cleaning = cfg["make_cleaning"][1]

    # 5) Готовим пути
    background_folder = f"b{background}"
    clean_info = 'sig' if use_sigma_for_cleaning else 'fix'
    clean_folder = f"{clean_edge1}-{clean_edge2}{clean_info}"
    back_path_folder = os.path.join(save_path0, background_folder)
    clean_path_folder = os.path.join(back_path_folder, clean_folder)

    # 6) Основная логика
    if make_trigger or make_background or make_cleaning:
        save_processing_info = os.path.join(save_path0, 'TAIGA_optics_info')
        os.makedirs(save_processing_info, exist_ok=True)

    if make_trigger:
        for filename in os.listdir(initial_dir):
            if "arameters" in filename:
                source_file = os.path.join(initial_dir, filename)
                destination_file = os.path.join(save_processing_info, filename)
                if os.path.exists(destination_file):
                    sys.exit(f"Триггер в {save_path0} уже смоделирован")
                shutil.copy2(source_file, destination_file)
                print(f"Файл {filename} скопирован в {save_path0}")
            if "config" in filename:
                source_file = os.path.join(initial_dir, filename)
                destination_file = os.path.join(save_processing_info, filename)
                shutil.copy2(source_file, destination_file)
                print(f"Файл {filename} скопирован в {save_path0}")
        # Сохраняем cfg с явным initial_file
        cfg_with_initial = dict(cfg)
        cfg_with_initial['initial_feb_file'] = initial_file
        config_trigger_path = os.path.join(save_path0, 'config_trigger.json')
        with open(config_trigger_path, 'w', encoding='utf-8') as f:
            json.dump(cfg_with_initial, f, ensure_ascii=False, indent=2)

        params = []
        for iact in IACTs:
            n_pix, n_clusters, config_path, max_sense = get_config(iact, coord_files[iact-1], factors_path[iact-1], use_sense, save_path0)
            tim0, max_amp, len_pulse = get_slow_pulse_const(pulse_files[iact-1])
            out_file = ''
            if (background != 2):
                out_file = os.path.join(save_path0,file_simulation[:-4] + '_iact0' + str(iact) + '_c.txt')
            elif (background == 2):
                out_file = os.path.join(back_path_folder,file_simulation[:-4] + '_iact0' + str(iact) + '_cb'+str(background) + '.txt')
            hidden_mirror[iact-1] = (np.array(hidden_mirror[iact-1])).flat
            if len(out_file)>0:
                #print(tim0,max_amp,pulse_files[iact-1])
                line_param = ['trigger_iact',iact, n_pix, n_clusters,trigger_type[iact-1], t_grid,
                        t_sign, trig_amp, t_hold, integrate_window[iact-1], trig_window,
                        t_after_before,len_pulse, tim0, max_amp, background,
                        background_params[2]['mean_ph'], background_params[2]['mean_phe_time'], config_path, amplitudes_file, out_file,
                        initial_file, pulse_files[iact-1], sense_cam[iact-1], len(hidden_mirror[iact-1])/2, *hidden_mirror[iact-1]]
                params.append(line_param)
        pool = multiprocessing.Pool()
        pool.map(launch_gcc_script, params)

    if make_background:
        back_path_folder = uniquify(back_path_folder)
        clean_path_folder = os.path.join(back_path_folder, clean_folder)
        os.makedirs(back_path_folder, exist_ok=False)
        if make_trigger == 0:
            config_trigger_path = os.path.join(back_path_folder, 'config_trigger.json')
            with open(config_trigger_path, 'w', encoding='utf-8') as f:
                json.dump(cfg, f, ensure_ascii=False, indent=2)
        if background !=2:
            background_line_params = []
            #print(background_params[1]['experimental_background_path'][1])
            clean_line_params = []
            for iact in IACTs:
                #if (make_trigger == 0):
                n_pix, n_clusters, config_path, max_sense = get_config(iact, coord_files[iact-1], factors_path[iact-1], use_sense, save_path0)
                in_background_file = os.path.join(save_path0,file_simulation[:-4] + '_iact0' + str(iact) + '_c.txt')
                out_background_file = os.path.join(back_path_folder,file_simulation[:-4] + '_iact0' + str(iact) + '_cb'+str(background) + '.txt')
                hist_sig_path_img = os.path.join(back_path_folder, 'sigma_hist.png')
                background_line_params.append([background,config_path, use_sense, factors_path[iact-1],
                                        in_background_file, out_background_file, trigger_type[iact-1], background_params[0]['distribution background'][iact]['mean'],
                                        background_params[0]['distribution background'][iact]['std'], background_params[0]['distribution background'][iact]['n'], background_params[1]['experimental_background_path'][iact], iact, hist_sig_path_img, coord_files, n_clusters, max_n_channels_per_cluster, max_sense])
                in_cleaning_file = os.path.join(back_path_folder,file_simulation[:-4] + '_iact0' + str(iact) + '_cb'+str(background) + '.txt')
                out_images_cleaning_file = os.path.join(clean_path_folder, file_simulation[:-4] + '_clean_iact0' + str(iact) + '_' + str(clean_edge1) + '_' + str(clean_edge2) + clean_info + '_cb' + str(background) + '.txt')
                out_hillas_cleaning_file = os.path.join(clean_path_folder, file_simulation[:-4] + '_hillas_iact0' + str(iact) + '_' + str(clean_edge1) + '_' + str(clean_edge2) + clean_info + '_cb' + str(background) + '.csv')
                clean_line_params.append(['cleaning', iact, gam, n_pix, clean_edge1, clean_edge2, config_path, in_cleaning_file, out_images_cleaning_file, out_hillas_cleaning_file, source_scatter_radius, factors_path[iact-1], sigma_for_clean[iact-1], use_sigma_for_cleaning, use_brightest_island])
        pool_obj1 = multiprocessing.Pool()
        sig_array = pool_obj1.map(add_background, background_line_params)

    if make_cleaning:
        if make_background:
            clean_path_folder = uniquify(clean_path_folder)
            os.makedirs(clean_path_folder, exist_ok=False)
        else:
            back_path_folder = os.path.join(save_path0, background_folder_for_cleaning)
            clean_path_folder = os.path.join(back_path_folder, clean_folder)
            clean_path_folder = uniquify(clean_path_folder)
            os.makedirs(clean_path_folder, exist_ok=False)
            config_trigger_path = os.path.join(clean_path_folder, 'config_trigger.json')
            with open(config_trigger_path, 'w', encoding='utf-8') as f:
                json.dump(cfg, f, ensure_ascii=False, indent=2)
            clean_line_params = []
            if background_folder_for_cleaning != "None":
                cleaning_dir = os.path.join(save_path0, background_folder_for_cleaning)
                in_cleaning_files = [os.path.join(cleaning_dir, f) for f in os.listdir(cleaning_dir) if f.endswith('.txt')]
                IACTs_mod = []
                for f in in_cleaning_files:
                    match = re.search(r'iact(\d+)', f)
                    if match:
                        IACTs_mod.append(int(match.group(1)))
                for iact in IACTs_mod:
                    n_pix, n_clusters, config_path, max_sense = get_config(iact, coord_files[iact-1], factors_path[iact-1], use_sense, save_path0)
                    out_images_cleaning_file = os.path.join(clean_path_folder, file_simulation[:-4] + '_clean_iact0' + str(iact) + '_' + str(clean_edge1) + '_' + str(clean_edge2) + clean_info + '_cb' + str(background) + '.txt')
                    out_hillas_cleaning_file = os.path.join(clean_path_folder, file_simulation[:-4] + '_hillas_iact0' + str(iact) + '_' + str(clean_edge1) + '_' + str(clean_edge2) + clean_info + '_cb' + str(background) + '.csv')
                    clean_line_params.append(['cleaning', iact, gam, n_pix, clean_edge1, clean_edge2, config_path, in_cleaning_files[iact-1], out_images_cleaning_file, out_hillas_cleaning_file, source_scatter_radius, factors_path[iact-1], sigma_for_clean[iact-1], use_sigma_for_cleaning, use_brightest_island])
            else:
                sys.exit("Укажите родительскую папку фона в make_cleaning")
        pool_obj2 = multiprocessing.Pool()
        pool_obj2.map(launch_gcc_script, clean_line_params)

if __name__ == "__main__":
    main()
