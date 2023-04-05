from skbuild import setup

setup(
    name="pylibmed",
    version="0.0.1",
    description="Data acquisition library for medical sensors",
    author='Nikita Travkin',
    packages=['pylibmed'],
    package_dir={"": "python"},
    cmake_install_dir="python/",
)
