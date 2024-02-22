#python -B -m pytest --rootdir=./src -p no:cacheprovider
import pytest
import shutil
import os
from src import main
import shutil
import subprocess
import argparse

data = os.path.join(os.path.dirname(os.path.abspath(__file__)), "data")
cia = "Castlevania Aria of Sorrow.cia"

@pytest.fixture(scope="function", autouse=True)
def my_fixture():
    print('\nINITIALIZATION')
    if os.path.exists(main.temp_dir):
        shutil.rmtree(main.temp_dir)
    if os.path.exists(os.path.join(os.getcwd(), "out")):
        shutil.rmtree(os.path.join(os.getcwd(), "out"))
    yield
    print('\nTEAR DOWN')
    if os.path.exists(main.temp_dir):
        shutil.rmtree(main.temp_dir)
    if os.path.exists(os.path.join(os.getcwd(), "out")):
        shutil.rmtree(os.path.join(os.getcwd(), "out"))

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

def test_game_class():
    cia2 = os.path.join(data, cia)

    game = main.Game(cia2)
    assert game.cia_path == cia2
    assert game.name == cia[:-4]
    assert game.cwd == os.path.join(main.temp_dir, game.name)
    assert game.version == (0, 6, 9)

    os.mkdir(main.temp_dir)
    assert os.path.exists(main.temp_dir)

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
    assert os.path.exists(os.path.join(os.getcwd(), "out", cia))

def test_cia_in_cwd(tmp_path):
    shutil.copy2(os.path.join(data, cia), os.getcwd())
    subprocess.run(["py", "src/main.py"])
    os.remove(os.path.join(os.getcwd(), cia))
    assert os.path.exists(os.path.join(os.getcwd(), "out", cia))

def test_get_cias_no_arg():
    shutil.copy2(os.path.join(data, cia), os.getcwd())
    parser = argparse.ArgumentParser()
    parser.add_argument("input", metavar = "input.cia", type = str, nargs = "*", help = "path to a .cia file")
    args = parser.parse_args([])
    files = main.get_cias(args)
    os.remove(os.path.join(os.getcwd(), cia))
    assert files == [os.path.join(os.getcwd(), cia)]

def test_get_cias_arg():
    parser = argparse.ArgumentParser()
    parser.add_argument("input", metavar = "input.cia", type = str, nargs = "*", help = "path to a .cia file")
    args = parser.parse_args([os.path.join(data, cia)])
    files = main.get_cias(args)
    assert files == [os.path.join(data, cia)]