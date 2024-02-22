#python -B -m pytest --rootdir=./src -p no:cacheprovider
import pytest
import shutil
import os
from src import main
import subprocess
import shutil

data = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")

@pytest.fixture(scope="session", autouse=True)
def my_fixture():
    print('\nINITIALIZATION')
    #shutil.c
    yield
    main.clean_dirs()
    print('\nTEAR DOWN')

def test_clean_dir():
    os.mkdir(main.temp_dir)
    main.clean_dirs()
    assert os.path.exists(main.temp_dir) == False

def test_check_requirements(capsys):
    main.check_requirements()
    captured = capsys.readouterr()
    assert captured.out == ""

def test_finish_msg(capsys):
    with pytest.raises(SystemExit):
        msg = "test msg that contains stuff 1 2 $ \"  l o "
        main.finish(msg)
    captured = capsys.readouterr()
    assert captured.out == "\n" + msg + "\n"

def test_finish_no_msg():
    with pytest.raises(SystemExit):
        main.finish()

def test_game_class_wrong_cia_path():
    with pytest.raises(subprocess.CalledProcessError):
        game = main.Game("wow.cia")

def test_game_class():
    #cia = os.path.join(main.script_dir, "Castlevania Aria of Sorrow.cia")
    cia = os.path.join(data, "Castlevania Aria of Sorrow.cia")
    game = main.Game(cia)
    
    main.clean_dirs()
    
def test_game_class_unpack_cia():
    if os.path.exists(main.temp_dir):
        shutil.rmtree(main.temp_dir)
    if os.path.exists(os.path.join(main.script_dir, "out")):
        shutil.rmtree(os.path.join(main.script_dir, "out"))

    cia = os.path.join(main.script_dir, "Castlevania Aria of Sorrow.cia")

    game = main.Game(cia)
    assert game.cia_path == cia
    assert game.name == "Castlevania Aria of Sorrow"
    assert game.cwd == os.path.join(main.temp_dir, game.name)
    assert game.version == (0, 6, 9)

    game.extract_cia()
    assert os.path.exists(os.path.join(game.cwd, "contents.0000.00000000"))
    assert os.path.exists(os.path.join(game.cwd, "ncch.header"))
    assert os.path.exists(os.path.join(game.cwd, "exheader.bin"))
    assert os.path.exists(os.path.join(game.cwd, "exefs.bin"))
    assert os.path.exists(os.path.join(game.cwd, "romfs.bin"))
    assert os.path.exists(os.path.join(game.cwd, "exefs.header"))
    assert os.path.exists(os.path.join(game.cwd, "exefs"))
    assert os.path.exists(os.path.join(game.cwd, "banner"))
    assert os.path.exists(os.path.join(game.cwd, "exefs", f"banner.{game.banner_ext}"))
    
    game.edit_bcmdl()
    for i in range(1, 14):
        with open(os.path.join(game.cwd, "banner", f"banner{i}.bcmdl"), "r+b") as f:
            f.seek(main.locale_offsets[0], 0)
            assert f.read(6) == main.locale_codes[i-1]
            f.seek(main.locale_offsets[1], 0)
            assert f.read(6) == main.locale_codes[i-1]

    game.repack_cia()
    assert os.path.exists(os.path.join(main.script_dir, "out", "Castlevania Aria of Sorrow.cia"))