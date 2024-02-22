#python -B -m pytest --rootdir=./src -p no:cacheprovider
import pytest
import shutil
import os
from src import main
import subprocess

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
