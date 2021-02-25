#-------------------------------------------------------------------------------
# Part of tweedledum.  This file is distributed under the MIT License.
# See accompanying file /LICENSE for details.
#-------------------------------------------------------------------------------
import sys

try:
    from skbuild import setup
except ImportError:
    print('scikit-build is required to build from source.', file=sys.stderr)
    print('Please run:', file=sys.stderr)
    print('', file=sys.stderr)
    print('  python -m pip install scikit-build')
    sys.exit(1)

setup(
    name="tweedledum",
    version="0.1",
    description="A library for synthesizing and manipulating quantum circuits",
    url="https://github.com/boschmitt/tweedledum",
    author='Bruno Schmitt',
    author_email='bruno.schmitt@epfl.ch',
    license="MIT",
    package_dir={'': 'python'},
    packages=['tweedledum', 'tweedledum.parser'],
    include_package_data=True,
    zip_safe=False,
    python_requires=">=3.5",
    classifiers=[
        "License :: OSI Approved :: MIT License",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: MacOS",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: 3.5",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8"
    ]
)