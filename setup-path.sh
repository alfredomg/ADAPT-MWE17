

rootMWE=$(pwd)

cd sem-reranker/context-features-code; make; cd ../..

export PATH=$PATH:$rootMWE/crf/scripts:$rootMWE/sem-reranker/context-features-code:$rootMWE/sem-reranker/scripts:$rootMWE/sem-reranker/third-party-software/organizers-bin/:$rootMWE/sem-reranker/third-party-software/erwans-scripts::$rootMWE/sem-reranker/third-party-software/parseme_tokenise

# weka.jar
export CLASSPATH=$CLASSPATH:$rootMWE/third-party-software/weka.jar

