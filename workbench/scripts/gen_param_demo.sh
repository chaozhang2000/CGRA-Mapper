cat > param.json << EOF
{
  "kernel"                : "_Z6kernelPiS_S_",
  "targetFunction"        : false,
  "targetNested"          : false,
  "doCGRAMapping"         : true,
  "row"                   : 4,
  "column"                : 4,
  "precisionAware"        : true,
  "isTrimmedDemo"         : true,
  "heuristicMapping"      : true,
  "parameterizableCGRA"   : false,
  "bypassConstraint"      : 4,
  "isStaticElasticCGRA"   : false,
  "ctrlMemConstraint"     : 200,
  "regConstraint"         : 8,
  "optLatency"            : {
                              "load" : 1,
                              "store": 1
                            },
  "optPipelined"          : ["load", "store"],
  "additionalFunc"        : {
                              "load" : [0,4,8,12],
                              "store": [0,4,8,12]
                            }
}
EOF
