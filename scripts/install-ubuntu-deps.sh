#!/usr/bin/env bash
set -euo pipefail

sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  ninja-build \
  libxkbcommon-dev \
  qt6-base-dev \
  qt6-declarative-dev \
  qt6-tools-dev \
  qml6-module-qtqml \
  qml6-module-qtqml-models \
  qml6-module-qtqml-workerscript \
  qml6-module-qtquick \
  qml6-module-qtquick-controls \
  qml6-module-qtquick-layouts \
  qml6-module-qtquick-templates \
  qml6-module-qtquick-window
