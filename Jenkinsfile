pipeline {
    agent any

    options {
        timestamps()
    }

    stages {
        stage('Checkout') {
            steps {
                echo 'Checkout stage (for demo, using current workspace source).'
            }
        }

        stage('Setup') {
            steps {
                sh 'python3 --version || true'
                sh 'pwd'
                sh 'ls -la'
            }
        }

        stage('Run Demo Script') {
            steps {
                dir('leaningProgram') {
                    sh 'python3 mysql_demo.py'
                }
            }
        }

        stage('Archive Logs') {
            steps {
                writeFile file: 'build-summary.txt', text: 'mysql_demo.py executed successfully in Jenkins pipeline.\n'
                archiveArtifacts artifacts: 'build-summary.txt', fingerprint: true
            }
        }
    }

    post {
        success {
            echo 'Pipeline finished successfully.'
        }
        failure {
            echo 'Pipeline failed. Check console output.'
        }
        always {
            echo 'Pipeline finished (success or failure).'
        }
    }
}
