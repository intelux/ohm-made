import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="ohm-made",
    version="0.0.7",
    author="Julien Kauffmann",
    author_email="julien.kauffmann@freelan.org",
    description="A library to control Ohm-Made devices.",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/pypa/sampleproject",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
    install_requires=[
        'aiohttp>=3.6.0',
        'aiodns>=2.0.0',
        'aiozeroconf>=0.1.8',
    ],
)
