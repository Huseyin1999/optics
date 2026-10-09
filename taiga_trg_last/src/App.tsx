import React, { useState } from 'react';
import { 
  GitBranch, 
  Terminal, 
  BookOpen, 
  FileText, 
  Sliders, 
  Layers, 
  Copy, 
  Check, 
  Cpu, 
  Zap, 
  Box, 
  Download, 
  ShieldCheck, 
  FolderArchive, 
  ArrowDownToLine, 
  AlertCircle,
  HelpCircle,
  Code2
} from 'lucide-react';

export default function App() {
  const [activeTab, setActiveTab] = useState<'download' | 'overview' | 'full' | 'quickstart' | 'generator'>('download');
  const [copiedId, setCopiedId] = useState<string | null>(null);

  // Command generator states
  const [corsikaDir, setCorsikaDir] = useState('/data/corsika');
  const [runDir, setRunDir] = useState('$HOME/taiga_runs/production01');
  const [radius, setRadius] = useState(100);
  const [keepFeb, setKeepFeb] = useState(false);
  const [limit, setLimit] = useState(1);
  const [pattern, setPattern] = useState('*_iact.corsika');
  const [amplitudesPath, setAmplitudesPath] = useState('/path/to/probablies8.txt');

  const copyToClipboard = (text: string, id: string) => {
    navigator.clipboard.writeText(text);
    setCopiedId(id);
    setTimeout(() => setCopiedId(null), 2000);
  };

  const unpackExistingTarCmd = `# Если файл уже скачан в ~/Downloads:\nfile ~/Downloads/optics_trg5_integration.tar.gz\n\n# Распаковка без ключа -z (браузер уже снял сжатие gzip):\ntar -xf ~/Downloads/optics_trg5_integration.tar.gz\ncd repo_optics\ngit status`;

  const applyPatchCmd = `# В папке вашего уже склонированного репозитория (/media/husein/Work_Hard/TAIGA/optics):\ncd /media/husein/Work_Hard/TAIGA/optics\n\n# Создать и переключиться на новую ветку:\ngit checkout -b feature/trg5-pipeline-integration\n\n# Применить скачанный патч:\ngit apply ~/Downloads/trg5_integration.patch\n\n# Проверить изменения:\ngit status`;

  const generatedRunCmd = `python3 prepare_run.py ${corsikaDir} "${runDir}" \\
  --radius ${radius} \\
  ${limit > 0 ? `--limit ${limit} \\
  ` : ''}${keepFeb ? `--keep-feb \\
  ` : ''}--pattern "${pattern}"`;

  const generatedPackCmd = `python3 pack_runtime.py \\
  --hybrid /path/to/optics/io_taiga/hybrid \\
  --optics /path/to/optics/condor_pipeline/TAIGA_optics_file \\
  --assets /path/to/optics/condor_pipeline/runtime/assets \\
  --trigger /path/to/optics/trg5_git/trigger_iact \\
  --cleaning /path/to/optics/trg5_git/cleaning \\
  --trg-scripts /path/to/optics/trg5_git \\
  --trg-assets /path/to/optics/trg5_git \\
  --amplitudes-file ${amplitudesPath} \\
  --output runtime.tar.gz`;

  return (
    <div className="min-h-screen bg-slate-950 text-slate-100 flex flex-col font-sans">
      {/* Top Header */}
      <header className="border-b border-slate-800 bg-slate-900/80 backdrop-blur sticky top-0 z-50">
        <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8 h-16 flex items-center justify-between">
          <div className="flex items-center space-x-3">
            <div className="w-9 h-9 rounded-lg bg-cyan-600/20 border border-cyan-500/40 flex items-center justify-center text-cyan-400">
              <Zap className="w-5 h-5" />
            </div>
            <div>
              <h1 className="text-base font-semibold leading-tight text-white flex items-center gap-2">
                TAIGA-IACT Condor Pipeline & Trigger
                <span className="text-xs px-2 py-0.5 rounded-full bg-emerald-500/20 text-emerald-400 border border-emerald-500/30 flex items-center gap-1 font-mono">
                  <GitBranch className="w-3 h-3" /> feature/trg5-pipeline-integration
                </span>
              </h1>
              <p className="text-xs text-slate-400">Сквозное моделирование: CORSIKA → hybrid → TAIGA_optics → trg5 → Hillas CSV</p>
            </div>
          </div>

          {/* Navigation tabs */}
          <nav className="flex items-center space-x-1 bg-slate-800/80 p-1 rounded-lg border border-slate-700/60 text-xs">
            <button
              onClick={() => setActiveTab('download')}
              className={`px-3 py-1.5 rounded-md font-medium transition-colors flex items-center gap-1.5 ${
                activeTab === 'download' ? 'bg-emerald-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              <ArrowDownToLine className="w-3.5 h-3.5" /> Скачать репозиторий
            </button>
            <button
              onClick={() => setActiveTab('overview')}
              className={`px-3 py-1.5 rounded-md font-medium transition-colors flex items-center gap-1.5 ${
                activeTab === 'overview' ? 'bg-cyan-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              <Layers className="w-3.5 h-3.5" /> Обзор цепочки
            </button>
            <button
              onClick={() => setActiveTab('full')}
              className={`px-3 py-1.5 rounded-md font-medium transition-colors flex items-center gap-1.5 ${
                activeTab === 'full' ? 'bg-cyan-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              <BookOpen className="w-3.5 h-3.5" /> README_FULL.md
            </button>
            <button
              onClick={() => setActiveTab('quickstart')}
              className={`px-3 py-1.5 rounded-md font-medium transition-colors flex items-center gap-1.5 ${
                activeTab === 'quickstart' ? 'bg-cyan-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              <Terminal className="w-3.5 h-3.5" /> README_QUICKSTART.md
            </button>
            <button
              onClick={() => setActiveTab('generator')}
              className={`px-3 py-1.5 rounded-md font-medium transition-colors flex items-center gap-1.5 ${
                activeTab === 'generator' ? 'bg-cyan-600 text-white shadow-sm' : 'text-slate-400 hover:text-white'
              }`}
            >
              <Sliders className="w-3.5 h-3.5" /> Конфигуратор
            </button>
          </nav>
        </div>
      </header>

      {/* Main Content Area */}
      <main className="flex-1 max-w-7xl w-full mx-auto px-4 sm:px-6 lg:px-8 py-6">
        {activeTab === 'download' && (
          <div className="space-y-6">
            {/* Direct Download Options */}
            <div className="p-6 rounded-2xl bg-gradient-to-br from-slate-900 via-slate-900 to-slate-950 border border-slate-800 shadow-xl space-y-4">
              <div>
                <span className="text-xs font-bold uppercase tracking-wider text-emerald-400 bg-emerald-950/80 px-2.5 py-1 rounded-full border border-emerald-800/60 inline-flex items-center gap-1.5 mb-2">
                  <Check className="w-3.5 h-3.5" /> Готовые файлы для скачивания
                </span>
                <h2 className="text-xl font-bold text-white">
                  Способы загрузки репозитория с веткой feature/trg5-pipeline-integration
                </h2>
                <p className="text-xs text-slate-400 mt-1 max-w-3xl">
                  Выберите наиболее удобный формат: компактный патч для вашего локального репозитория, стандартный ZIP-архив без проблем с распаковкой или команду для уже скачанного tar.gz.
                </p>
              </div>

              {/* Download Buttons Bar */}
              <div className="flex flex-wrap items-center gap-3 pt-2">
                {/* 1. Git Patch */}
                <a
                  href="/trg5_integration.patch"
                  download="trg5_integration.patch"
                  className="px-4 py-2.5 rounded-xl bg-indigo-600 hover:bg-indigo-500 text-white font-semibold text-xs transition-colors flex items-center gap-2 shadow-lg shadow-indigo-900/20"
                >
                  <Code2 className="w-4 h-4" /> Скачать Git Patch (.patch, 87 КБ)
                </a>

                {/* 2. ZIP Archive */}
                <a
                  href="/optics_trg5_integration.zip"
                  download="optics_trg5_integration.zip"
                  className="px-4 py-2.5 rounded-xl bg-emerald-600 hover:bg-emerald-500 text-white font-semibold text-xs transition-colors flex items-center gap-2 shadow-lg shadow-emerald-900/20"
                >
                  <FolderArchive className="w-4 h-4" /> Скачать ZIP архив (.zip, 35 МБ)
                </a>

                {/* 3. Pure TAR Archive */}
                <a
                  href="/optics_trg5_integration.tar"
                  download="optics_trg5_integration.tar"
                  className="px-4 py-2.5 rounded-xl bg-cyan-600 hover:bg-cyan-500 text-white font-semibold text-xs transition-colors flex items-center gap-2 shadow-lg shadow-cyan-900/20"
                >
                  <Box className="w-4 h-4" /> Скачать TAR архив (.tar, 36 МБ)
                </a>
              </div>
            </div>

            {/* Error explanations and immediate fixes */}
            <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
              {/* Fix 1: Why tar -xzf failed */}
              <div className="p-5 rounded-2xl bg-slate-900 border border-amber-500/30 flex flex-col justify-between space-y-4">
                <div>
                  <div className="flex items-center justify-between">
                    <span className="text-xs font-bold text-amber-400 flex items-center gap-1.5">
                      <AlertCircle className="w-4 h-4" /> Решение для ошибки: "gzip: stdin: not in gzip format"
                    </span>
                    <button
                      onClick={() => copyToClipboard(unpackExistingTarCmd, 'cmd-unpack')}
                      className="text-slate-400 hover:text-white p-1 rounded"
                      title="Копировать команду"
                    >
                      {copiedId === 'cmd-unpack' ? <Check className="w-3.5 h-3.5 text-emerald-400" /> : <Copy className="w-3.5 h-3.5" />}
                    </button>
                  </div>
                  <p className="text-xs text-slate-300 mt-2 leading-relaxed">
                    <strong>Причина:</strong> Ваш браузер при скачивании автоматически распаковал сжатие gzip на лету. Файл в папке Downloads уже является чистым <code>.tar</code>, поэтому ключ <code>-z</code> (повторный gzip) выдаёт ошибку.
                  </p>
                  <p className="text-xs text-slate-400 mt-1">
                    <strong>Решение:</strong> распакуйте без флага <code>-z</code>:
                  </p>
                  <pre className="mt-3 p-3 rounded-xl bg-slate-950 border border-slate-800 text-[11px] font-mono text-emerald-400 overflow-x-auto whitespace-pre leading-relaxed">
{unpackExistingTarCmd}
                  </pre>
                </div>
                <div className="text-[11px] text-slate-500 font-sans">
                  Либо скачайте выше готовый файл <strong>optics_trg5_integration.zip</strong> и распакуйте командой <code>unzip optics_trg5_integration.zip</code>.
                </div>
              </div>

              {/* Fix 2: Git Patch for existing clone */}
              <div className="p-5 rounded-2xl bg-slate-900 border border-indigo-500/30 flex flex-col justify-between space-y-4">
                <div>
                  <div className="flex items-center justify-between">
                    <span className="text-xs font-bold text-indigo-400 flex items-center gap-1.5">
                      <Code2 className="w-4 h-4" /> Самый надёжный способ: Применить Git Patch
                    </span>
                    <button
                      onClick={() => copyToClipboard(applyPatchCmd, 'cmd-patch')}
                      className="text-slate-400 hover:text-white p-1 rounded"
                      title="Копировать команду"
                    >
                      {copiedId === 'cmd-patch' ? <Check className="w-3.5 h-3.5 text-emerald-400" /> : <Copy className="w-3.5 h-3.5" />}
                    </button>
                  </div>
                  <p className="text-xs text-slate-300 mt-2 leading-relaxed">
                    Так как у вас уже есть папка с репозиторием (<code>/media/husein/Work_Hard/TAIGA/optics</code>), просто примените патч <strong>trg5_integration.patch</strong> прямо в нём:
                  </p>
                  <pre className="mt-3 p-3 rounded-xl bg-slate-950 border border-slate-800 text-[11px] font-mono text-indigo-300 overflow-x-auto whitespace-pre leading-relaxed">
{applyPatchCmd}
                  </pre>
                </div>
                <div className="text-[11px] text-slate-500 font-sans">
                  Это создаст ветку <code>feature/trg5-pipeline-integration</code> со всеми 11 файлами и 100% совместимостью.
                </div>
              </div>
            </div>

            {/* Explanation regarding curl and bundle */}
            <div className="p-5 rounded-2xl bg-slate-900/80 border border-slate-800 text-xs text-slate-400 space-y-2">
              <div className="flex items-center gap-2 text-slate-200 font-semibold">
                <HelpCircle className="w-4 h-4 text-cyan-400" /> Почему при curl bundle возникла ошибка: "does not look like a v2 or v3 bundle file"?
              </div>
              <p>
                Превью-сервер приложения защищён авторизацией Google AI Studio. Когда вы выполняете <code>curl</code> в терминале без сессионных cookie Google, сервер возвращает HTML-страницу авторизации. Соответственно, <code>git clone</code> пытается прочитать HTML вместо бинарного бандла.
              </p>
              <p>
                <strong>Правильный путь:</strong> скачивайте файлы кнопками в браузере (где сессия авторизована), либо используйте <strong>trg5_integration.patch</strong>!
              </p>
            </div>
          </div>
        )}

        {activeTab === 'overview' && (
          <div className="space-y-6">
            {/* Highlights Banner */}
            <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
              <div className="p-4 rounded-xl bg-slate-900 border border-slate-800 flex items-start space-x-3.5">
                <div className="p-2.5 rounded-lg bg-emerald-500/10 border border-emerald-500/20 text-emerald-400 mt-0.5">
                  <ShieldCheck className="w-5 h-5" />
                </div>
                <div>
                  <h3 className="text-sm font-semibold text-slate-200">Изоляция в Git</h3>
                  <p className="text-xs text-slate-400 mt-1">
                    Основная ветка <code className="text-cyan-400 bg-slate-800 px-1 py-0.5 rounded">main</code> сохранена нетронутой. Все изменения закоммичены в <code className="text-emerald-400 bg-slate-800 px-1 py-0.5 rounded">feature/trg5-pipeline-integration</code>.
                  </p>
                </div>
              </div>

              <div className="p-4 rounded-xl bg-slate-900 border border-slate-800 flex items-start space-x-3.5">
                <div className="p-2.5 rounded-lg bg-cyan-500/10 border border-cyan-500/20 text-cyan-400 mt-0.5">
                  <Box className="w-5 h-5" />
                </div>
                <div>
                  <h3 className="text-sm font-semibold text-slate-200">Экономия диска (--keep-feb)</h3>
                  <p className="text-xs text-slate-400 mt-1">
                    Бинарный FEB файл по умолчанию удаляется на воркере после работы триггера. На submit возвращаются только физические таблицы Hillas и архив событий.
                  </p>
                </div>
              </div>

              <div className="p-4 rounded-xl bg-slate-900 border border-slate-800 flex items-start space-x-3.5">
                <div className="p-2.5 rounded-lg bg-amber-500/10 border border-amber-500/20 text-amber-400 mt-0.5">
                  <FileText className="w-5 h-5" />
                </div>
                <div>
                  <h3 className="text-sm font-semibold text-slate-200">Две версии документации</h3>
                  <p className="text-xs text-slate-400 mt-1">
                    Подготовлены <span className="text-slate-200 font-medium">README_FULL.md</span> с полной теорией и <span className="text-slate-200 font-medium">README_QUICKSTART.md</span> с готовыми командами и рецептом контейнера.
                  </p>
                </div>
              </div>
            </div>

            {/* Visual Pipeline Flow */}
            <div className="p-6 rounded-2xl bg-slate-900/90 border border-slate-800 shadow-xl">
              <h2 className="text-base font-semibold text-slate-100 flex items-center gap-2 mb-4">
                <Cpu className="w-5 h-5 text-cyan-400" /> Сквозная цепочка исполнения на рабочем узле HTCondor
              </h2>

              <div className="grid grid-cols-1 md:grid-cols-5 gap-3 relative">
                <div className="p-4 rounded-xl bg-slate-800/60 border border-slate-700/60 flex flex-col justify-between">
                  <div>
                    <span className="text-[10px] font-bold uppercase tracking-wider text-cyan-400 bg-cyan-950/80 px-2 py-0.5 rounded border border-cyan-800/60">
                      Стадия 1
                    </span>
                    <h4 className="text-sm font-semibold text-slate-200 mt-2">CORSIKA → hybrid</h4>
                    <p className="text-xs text-slate-400 mt-1">
                      Разделение черенковских фотонов по радиусу (100 см) вокруг оси телескопа.
                    </p>
                  </div>
                  <div className="mt-4 pt-2 border-t border-slate-700/40 text-[11px] text-slate-300 font-mono">
                    Выход: <span className="text-cyan-300">intermediate_i</span>
                  </div>
                </div>

                <div className="p-4 rounded-xl bg-slate-800/60 border border-slate-700/60 flex flex-col justify-between">
                  <div>
                    <span className="text-[10px] font-bold uppercase tracking-wider text-indigo-400 bg-indigo-950/80 px-2 py-0.5 rounded border border-indigo-800/60">
                      Стадия 2
                    </span>
                    <h4 className="text-sm font-semibold text-slate-200 mt-2">TAIGA_optics</h4>
                    <p className="text-xs text-slate-400 mt-1">
                      Трассировка лучей через зеркала Дэвиса-Коттона, конусы Винстона и квантовую эффективность ФЭУ.
                    </p>
                  </div>
                  <div className="mt-4 pt-2 border-t border-slate-700/40 text-[11px] text-slate-300 font-mono">
                    Выход: <span className="text-indigo-300">result_feb</span>
                  </div>
                </div>

                <div className="p-4 rounded-xl bg-slate-800/60 border border-slate-700/60 flex flex-col justify-between">
                  <div>
                    <span className="text-[10px] font-bold uppercase tracking-wider text-amber-400 bg-amber-950/80 px-2 py-0.5 rounded border border-amber-800/60">
                      Стадия 3
                    </span>
                    <h4 className="text-sm font-semibold text-slate-200 mt-2">trigger_iact</h4>
                    <p className="text-xs text-slate-400 mt-1">
                      Спектр амплитуд XP1911 (probablies8.txt), сетка 1 нс, порог 10 ф.э., окно совпадений 15 нс.
                    </p>
                  </div>
                  <div className="mt-4 pt-2 border-t border-slate-700/40 text-[11px] text-slate-300 font-mono">
                    Выход: <span className="text-amber-300">iact0X_c.txt</span>
                  </div>
                </div>

                <div className="p-4 rounded-xl bg-slate-800/60 border border-slate-700/60 flex flex-col justify-between">
                  <div>
                    <span className="text-[10px] font-bold uppercase tracking-wider text-purple-400 bg-purple-950/80 px-2 py-0.5 rounded border border-purple-800/60">
                      Стадия 4
                    </span>
                    <h4 className="text-sm font-semibold text-slate-200 mt-2">NSB Background</h4>
                    <p className="text-xs text-slate-400 mt-1">
                      Наложение шума ночного неба со средней сигмой ~2.5–5.0 ф.э. в каждом пикселе.
                    </p>
                  </div>
                  <div className="mt-4 pt-2 border-t border-slate-700/40 text-[11px] text-slate-300 font-mono">
                    Выход: <span className="text-purple-300">iact0X_cb0.txt</span>
                  </div>
                </div>

                <div className="p-4 rounded-xl bg-slate-800/60 border border-slate-700/60 flex flex-col justify-between">
                  <div>
                    <span className="text-[10px] font-bold uppercase tracking-wider text-emerald-400 bg-emerald-950/80 px-2 py-0.5 rounded border border-emerald-800/60">
                      Стадия 5
                    </span>
                    <h4 className="text-sm font-semibold text-slate-200 mt-2">cleaning & Hillas</h4>
                    <p className="text-xs text-slate-400 mt-1">
                      Двухпороговый клининг 14/7 ф.э., выделение ярчайшего острова, вычисление моментов формы.
                    </p>
                  </div>
                  <div className="mt-4 pt-2 border-t border-slate-700/40 text-[11px] text-slate-300 font-mono">
                    Выход: <span className="text-emerald-300">_hillas.csv, _clean.txt</span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        )}

        {activeTab === 'full' && (
          <div className="p-6 rounded-2xl bg-slate-900 border border-slate-800 shadow-xl space-y-4">
            <div className="flex items-center justify-between pb-4 border-b border-slate-800">
              <div>
                <h2 className="text-base font-semibold text-white flex items-center gap-2">
                  <BookOpen className="w-5 h-5 text-cyan-400" /> condor_pipeline/README_FULL.md
                </h2>
                <p className="text-xs text-slate-400 mt-0.5">Полное физическое и алгоритмическое описание всех звеньев конвейера</p>
              </div>
            </div>
            <div className="p-4 rounded-xl bg-slate-950 border border-slate-800 text-slate-300 space-y-3 text-xs leading-relaxed font-mono">
              <h3 className="text-sm font-bold text-cyan-300 font-sans">Ключевые физические разделы руководства:</h3>
              <ul className="list-disc pl-5 space-y-2">
                <li><strong className="text-slate-100">1. CORSIKA → hybrid:</strong> Пространственный фильтр радиусом 100 см вокруг оптической оси телескопа TAIGA-IACT, разделение на _i и _t.</li>
                <li><strong className="text-slate-100">2. Оптическая трассировка (TAIGA_optics):</strong> Сегментированное зеркало Дэвиса-Коттона, конусы Винстона, спектральная квантовая эффективность ФЭУ XP1911. Выходной формат .feb.</li>
                <li><strong className="text-slate-100">3. Электроника и триггер (trg5):</strong> Одноэлектронные спектры XP1911 с послеимпульсами из probablies8.txt, временная сетка 1 нс, порог 10 ф.э., окно совпадений 15 нс, кластерный холд 160 нс, медленный формирователь с интерполяцией Акимы и интегрированием 80 нс.</li>
                <li><strong className="text-slate-100">4. Ночной фон неба (NSB):</strong> Флуктуации фона с сигмой ~2.5–5.0 ф.э. в зависимости от состояния телескопа и сезона.</li>
                <li><strong className="text-slate-100">5. Клининг и параметры Хилласа:</strong> Двухпороговый метод 14/7 ф.э., фильтрация островов, вычисление параметров формы (Size, Length, Width, Dist, Alpha, Azwidth, Miss).</li>
              </ul>
            </div>
          </div>
        )}

        {activeTab === 'quickstart' && (
          <div className="p-6 rounded-2xl bg-slate-900 border border-slate-800 shadow-xl space-y-4">
            <div className="flex items-center justify-between pb-4 border-b border-slate-800">
              <div>
                <h2 className="text-base font-semibold text-white flex items-center gap-2">
                  <Terminal className="w-5 h-5 text-emerald-400" /> condor_pipeline/README_QUICKSTART.md
                </h2>
                <p className="text-xs text-slate-400 mt-0.5">Краткая инструкция: компиляция, сборка архива, контейнер и отправка в HTCondor</p>
              </div>
            </div>

            <div className="space-y-4 text-xs font-mono">
              <div className="p-4 rounded-xl bg-slate-950 border border-slate-800 space-y-2">
                <div className="text-slate-400 text-[11px] font-sans font-medium">1. Сборка архива runtime.tar.gz с файлом probablies8.txt:</div>
                <pre className="text-slate-300 overflow-x-auto p-2 bg-slate-900/60 rounded border border-slate-800/60">
                  {generatedPackCmd}
                </pre>
              </div>

              <div className="p-4 rounded-xl bg-slate-950 border border-slate-800 space-y-2">
                <div className="text-slate-400 text-[11px] font-sans font-medium">2. Сборка контейнера Apptainer/Singularity (EL9 / Rocky Linux 9):</div>
                <pre className="text-slate-300 overflow-x-auto p-2 bg-slate-900/60 rounded border border-slate-800/60">
apptainer build taiga_runtime.sif container/taiga_runtime.def
                </pre>
              </div>

              <div className="p-4 rounded-xl bg-slate-950 border border-slate-800 space-y-2">
                <div className="text-slate-400 text-[11px] font-sans font-medium">3. Тестовый запуск на 1 событии и массовый запуск в HTCondor:</div>
                <pre className="text-slate-300 overflow-x-auto p-2 bg-slate-900/60 rounded border border-slate-800/60">
# Тест 1 задачи:
python3 prepare_run.py /data/corsika "$HOME/taiga_runs/test01" --limit 1
cd "$HOME/taiga_runs/test01"
condor_submit pipeline.sub

# Проверка очереди:
condor_q
tail -f logs/*-0.out
                </pre>
              </div>
            </div>
          </div>
        )}

        {activeTab === 'generator' && (
          <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
            <div className="p-6 rounded-2xl bg-slate-900 border border-slate-800 shadow-xl space-y-4">
              <h2 className="text-base font-semibold text-white flex items-center gap-2">
                <Sliders className="w-5 h-5 text-cyan-400" /> Настройка параметров запуска кампании
              </h2>

              <div className="space-y-3.5 text-xs">
                <div>
                  <label className="block text-slate-400 font-medium mb-1">Каталог с CORSIKA-файлами (input_dir)</label>
                  <input
                    type="text"
                    value={corsikaDir}
                    onChange={(e) => setCorsikaDir(e.target.value)}
                    className="w-full px-3 py-2 rounded-lg bg-slate-950 border border-slate-700 text-slate-200 focus:outline-none focus:border-cyan-500 font-mono"
                  />
                </div>

                <div>
                  <label className="block text-slate-400 font-medium mb-1">Каталог кампании (run_dir)</label>
                  <input
                    type="text"
                    value={runDir}
                    onChange={(e) => setRunDir(e.target.value)}
                    className="w-full px-3 py-2 rounded-lg bg-slate-950 border border-slate-700 text-slate-200 focus:outline-none focus:border-cyan-500 font-mono"
                  />
                </div>

                <div className="grid grid-cols-2 gap-3">
                  <div>
                    <label className="block text-slate-400 font-medium mb-1">Радиус отбора (--radius, см)</label>
                    <input
                      type="number"
                      value={radius}
                      onChange={(e) => setRadius(Number(e.target.value))}
                      className="w-full px-3 py-2 rounded-lg bg-slate-950 border border-slate-700 text-slate-200 focus:outline-none focus:border-cyan-500 font-mono"
                    />
                  </div>

                  <div>
                    <label className="block text-slate-400 font-medium mb-1">Лимит задач (--limit, 0 = все)</label>
                    <input
                      type="number"
                      value={limit}
                      onChange={(e) => setLimit(Number(e.target.value))}
                      className="w-full px-3 py-2 rounded-lg bg-slate-950 border border-slate-700 text-slate-200 focus:outline-none focus:border-cyan-500 font-mono"
                    />
                  </div>
                </div>

                <div>
                  <label className="block text-slate-400 font-medium mb-1">Маска входных файлов (--pattern)</label>
                  <input
                    type="text"
                    value={pattern}
                    onChange={(e) => setPattern(e.target.value)}
                    className="w-full px-3 py-2 rounded-lg bg-slate-950 border border-slate-700 text-slate-200 focus:outline-none focus:border-cyan-500 font-mono"
                  />
                </div>

                <div>
                  <label className="block text-slate-400 font-medium mb-1">Путь к файлу амплитуд XP1911 (--amplitudes-file)</label>
                  <input
                    type="text"
                    value={amplitudesPath}
                    onChange={(e) => setAmplitudesPath(e.target.value)}
                    className="w-full px-3 py-2 rounded-lg bg-slate-950 border border-slate-700 text-slate-200 focus:outline-none focus:border-cyan-500 font-mono"
                  />
                </div>

                <div className="pt-2">
                  <label className="flex items-center space-x-2.5 cursor-pointer">
                    <input
                      type="checkbox"
                      checked={keepFeb}
                      onChange={(e) => setKeepFeb(e.target.checked)}
                      className="w-4 h-4 rounded bg-slate-950 border-slate-700 text-cyan-600 focus:ring-cyan-500"
                    />
                    <span className="text-slate-300 font-medium">
                      Сохранять бинарный FEB файл (<code className="text-cyan-400">--keep-feb</code>)
                    </span>
                  </label>
                  <p className="text-[11px] text-slate-500 mt-0.5 ml-6">
                    По умолчанию выключено: FEB удаляется после триггера, возвращаются только Hillas и архив trg.
                  </p>
                </div>
              </div>
            </div>

            <div className="p-6 rounded-2xl bg-slate-900 border border-slate-800 shadow-xl space-y-4 flex flex-col justify-between">
              <div>
                <h2 className="text-base font-semibold text-white flex items-center gap-2 mb-3">
                  <Terminal className="w-5 h-5 text-emerald-400" /> Сгенерированная команда для HTCondor
                </h2>

                <div className="p-4 rounded-xl bg-slate-950 border border-slate-800 space-y-3 font-mono text-xs">
                  <div className="flex items-center justify-between text-slate-400 text-[11px] font-sans">
                    <span>Подготовка кампании (prepare_run.py):</span>
                    <button
                      onClick={() => copyToClipboard(generatedRunCmd, 'gen-cmd')}
                      className="text-cyan-400 hover:text-cyan-300 flex items-center gap-1 font-sans"
                    >
                      {copiedId === 'gen-cmd' ? <Check className="w-3.5 h-3.5 text-emerald-400" /> : <Copy className="w-3.5 h-3.5" />}
                      Копировать
                    </button>
                  </div>
                  <pre className="text-emerald-400 overflow-x-auto whitespace-pre-wrap leading-relaxed">
                    {generatedRunCmd}
                  </pre>
                </div>
              </div>

              <div className="pt-4 border-t border-slate-800 text-xs text-slate-400 flex items-center justify-between">
                <span>Запуск после генерации:</span>
                <code className="text-cyan-300 bg-slate-950 px-2 py-1 rounded border border-slate-800">
                  cd "{runDir}" && condor_submit pipeline.sub
                </code>
              </div>
            </div>
          </div>
        )}
      </main>

      {/* Footer */}
      <footer className="border-t border-slate-800/80 bg-slate-950 py-4 text-center text-xs text-slate-500">
        TAIGA-IACT Collaboration &middot; HTCondor Simulation Pipeline &middot; Branch: feature/trg5-pipeline-integration
      </footer>
    </div>
  );
}
