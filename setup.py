#-------------------------------------------------------------------------------
# Part of Tweedledum Project.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import os
import sys
import setuptools

try:
    import skbuild
except ImportError:
    print('scikit-build is required to build from source.', file=sys.stderr)
    print('Please run:', file=sys.stderr)
    print('', file=sys.stderr)
    print('  python -m pip install scikit-build')
    sys.exit(1)

README_PATH = os.path.join(os.path.abspath(os.path.dirname(__file__)),
                           'README.md')
with open(README_PATH) as readme_file:
    README = readme_file.read()

skbuild.setup(
    name="tweedledum",
    version="1.0.0-beta2",
    description="A library for synthesizing and manipulating quantum circuits",
    long_description=README,
    long_description_content_type='text/markdown',
    url="https://github.com/boschmitt/tweedledum",
    author='Bruno Schmitt',
    author_email='bruno.schmitt@epfl.ch',
    license="MIT",
    package_dir={'': 'python'},
    packages=setuptools.find_packages(where='python'),
    include_package_data=True,
    zip_safe=False,
    python_requires=">=3.6",
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: MacOS",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9"
    ]
)