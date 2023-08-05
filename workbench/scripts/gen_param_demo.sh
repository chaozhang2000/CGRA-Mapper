cat > param.json << EOF
{
  "kernel"                : "_Z6kernelPiS_S_",
  "targetFunction"        : false,
  "targetNested"          : true,
  "targetLoopsID"         : [0],
  "doCGRAMapping"         : true,
  "row"                   : 4,
  "column"                : 4,
  "precisionAware"        : true,
  "heterogeneity"         : false,
  "isTrimmedDemo"         : true,
  "heuristicMapping"      : true,
  "parameterizableCGRA"   : false,
  "diagonalVectorization" : false,
  "bypassConstraint"      : 4,
  "isStaticElasticCGRA"   : false,
  "ctrlMemConstraint"     : 200,
  "regConstraint"         : 8,
  "optLatency"            : {
                              "load" : 2,
                              "store": 2
                            },
  "optPipelined"          : ["load", "store"],
  "additionalFunc"        : {
                              "load" : [4],
                              "store": [4]
                            }
}
EOF
