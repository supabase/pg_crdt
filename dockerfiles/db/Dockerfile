FROM postgres:17
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update

RUN apt-get install -y --no-install-recommends ca-certificates \
  git \
  build-essential \
  libpq-dev \
  postgresql-server-dev-17 \
  postgresql-17-dbgsym \
  curl \
  libreadline6-dev \
  zlib1g-dev \
  cmake \
  doxygen \
  ninja-build \
  pkg-config \
  libssl-dev \
  libclang-dev \
  gdbserver \
  gdb \
  sudo 

WORKDIR /home/app

ENV HOME=/home/app \
  PATH=/home/app/.cargo/bin:/home/linuxbrew/.linuxbrew/bin:$PATH
    
RUN chown postgres:postgres /home/app
RUN usermod -aG sudo postgres
RUN echo "postgres ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers
    
USER postgres

RUN git clone https://github.com/automerge/automerge
RUN cd automerge

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

RUN /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

RUN test -d /home/linuxbrew/.linuxbrew && eval "$(/home/linuxbrew/.linuxbrew/bin/brew shellenv)"
RUN echo "eval \"\$($(brew --prefix)/bin/brew shellenv)\"" >> ~/.bashrc

RUN brew install node cmocka

RUN npm install --global yarn

RUN mkdir javascript
RUN yarn --verbose --cwd ./javascript

RUN cargo install wasm-bindgen-cli wasm-opt cargo-deny

RUN rustup toolchain install nightly
RUN rustup component add rust-src --toolchain nightly

RUN rustup target add wasm32-unknown-unknown

RUN cd /home/app/automerge/rust
RUN cmake -E make_directory /home/app/automerge/rust/automerge-c/build
RUN cmake -S /home/app/automerge/rust/automerge-c -B /home/app/automerge/rust/automerge-c/build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
RUN cmake --build /home/app/automerge/rust/automerge-c/build 
RUN sudo cmake --install /home/app/automerge/rust/automerge-c/build --prefix "/usr"

RUN cd ../..
COPY . .
RUN sudo make && sudo make install
