import * as THREE from "three";
import ReactDOM from "react-dom";
import React, { useRef, useState } from "react";
import { Canvas, useFrame } from "@react-three/fiber";
import type { Vector3 } from "@react-three/fiber";

function Box(props: JSX.IntrinsicElements["mesh"]) {
  const ref = useRef<THREE.Mesh>(null!);
  return (
    <mesh {...props} ref={ref}>
      <boxGeometry args={[1, 1, 1]} />
      <meshStandardMaterial color={"green"} />
    </mesh>
  );
}

type PlotProps = {
  acceleration: {
    x: number;
    y: number;
    z: number;
  };
};

const cameraOptions = {
  zoom: 1.5,
  position: new THREE.Vector3(1.5, 3, 1.5),
  up: new THREE.Vector3(0, 0, 1),
};

const Plot = ({ acceleration }: PlotProps) => {
  let { x, y, z } = acceleration;

  const rotation = new THREE.Euler(
    x * Math.PI,
    y * Math.PI,
    z * Math.PI,
    "XYZ"
  );

  return (
    <Canvas camera={cameraOptions}>
      <axesHelper />
      <ambientLight />
      <pointLight position={[10, 5, 1]} />
      <Box position={[0, 0, 0]} rotation={rotation} />
    </Canvas>
  );
};

export default Plot;
