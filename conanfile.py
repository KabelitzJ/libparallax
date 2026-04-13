import os
import shutil
import subprocess
from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeDeps, CMakeToolchain, CMake, cmake_layout
from conan.tools.files import copy, mkdir
from conan.tools.system.package_manager import Apt

class parallax_recipe(ConanFile):

  name = "parallax"
  version = "0.1.0"
  license = "MIT"
  author = "Jonas Kabelitz <jonas-kabelitz@gmx.de>"
  description = "parallax library"
  topics = (
    "chess",
    "lib"
  )
  settings = (
    "os", 
    "compiler", 
    "build_type", 
    "arch"
  )
  options = {
    "shared": [True, False],
    "fPIC": [True, False],
    "build_demo": [True, False],
    "build_tests": [True, False],
    "build_benchmarks": [True, False]
  }
  default_options = {
    "shared": False,
    "fPIC": True,
    "build_demo": True,
    "build_tests": True,
    "build_benchmarks": True
  }

  def config_options(self):
    if self.settings.os == "Windows":
      self.options.fPIC = False

  def layout(self):
    cmake_layout(self)

    is_compiler_multi_config = self.settings.compiler == "msvc"

    self.folders.build = os.path.join("build", str(self.settings.arch), str(self.settings.compiler))

    if not is_compiler_multi_config:
      self.folders.build = os.path.join(self.folders.build, str(self.settings.build_type).lower())

    self.folders.generators = os.path.join(self.folders.build, "dependencies")

  def build_requirements(self):
    self.tool_requires("cmake/[>=3.20]")

  def requirements(self):
    self.requires("fmt/11.2.0")
    self.requires("spdlog/1.15.3")
    self.requires("yaml-cpp/0.7.0")
    self.requires("nlohmann_json/3.11.3")
    self.requires("range-v3/0.12.0")
    self.requires("easy_profiler/2.1.0")
    self.requires("magic_enum/0.9.7")

    if self.options.build_tests:
      self.test_requires("gtest/1.17.0")

    if self.options.build_benchmarks:
      self.test_requires("benchmark/1.9.4")

  def generate(self):
    deps = CMakeDeps(self)
    toolchain = CMakeToolchain(self)

    deps.generate()
    toolchain.generate()

  def build(self):
    cmake = CMake(self)

    cmake.configure({
      "PARALLAX_BUILD_DEMO": "ON" if self.options.build_demo else "OFF",
      "PARALLAX_BUILD_SHARED": "ON" if self.options.shared else "OFF",
      "PARALLAX_BUILD_TESTS": "ON" if self.options.build_tests else "OFF",
      "PARALLAX_BUILD_BENCHMARKS": "ON" if self.options.build_benchmarks else "OFF"
    })
  
    cmake.build()

  def package(self):
    cmake = CMake(self)
    cmake.install()
